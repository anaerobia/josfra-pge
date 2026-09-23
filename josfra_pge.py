"""JOSFRA PGE demo algorithm for MAAP DPS.

Reads a JOSFRA-style XML config file and produces a NetCDF product,
a CAS metadata file, and a processing log.
"""

from __future__ import annotations

import argparse
import logging
import random
import string
import subprocess
import tempfile
import urllib.request
import xml.etree.ElementTree as ET
from datetime import datetime, timedelta
from pathlib import Path
from urllib.parse import urlparse
from xml.sax.saxutils import escape

import boto3
from netCDF4 import Dataset

OUTPUT_DIR = Path("output")
RANDOM_STRING_LENGTH = 5000
EXECUTABLE_DIR = Path(__file__).resolve().parent
GET_BUILD_ID_PATH = EXECUTABLE_DIR / "getBuildId"
TAI_TO_UTC_PATH = EXECUTABLE_DIR / "taiToUtc"
TAI_TO_UTC_ARGS = ["01"]

CONFIG_TIME_FORMAT = "%Y-%m-%dT%H:%M:%S.%fZ"
PRODUCTION_TIMESTAMP_FORMAT = "%y%m%d%H%M%S"
CAS_TIME_FORMAT = "%Y-%m-%dT%H:%M:%S.000Z"
CAS_DATE_FORMAT = "%Y-%m-%d"
CAS_NAMESPACE = "http://oodt.jpl.nasa.gov/1.0/cas"
GRANULE_DURATION_MINUTES = 6


def get_scalar(root: ET.Element, group_name: str, scalar_name: str) -> str:
    """Return the text value of a named scalar within a named group.

    Args:
        root: Root element of the parsed config XML.
        group_name: Name attribute of the containing <group> element.
        scalar_name: Name attribute of the target <scalar> element.

    Returns:
        The scalar's text value.

    Raises:
        ValueError: If the group or scalar cannot be found.
    """
    group = root.find(f"group[@name='{group_name}']")
    if group is None:
        raise ValueError(f"Group not found in config: {group_name}")
    scalar = group.find(f"scalar[@name='{scalar_name}']")
    if scalar is None or scalar.text is None:
        raise ValueError(f"Scalar '{scalar_name}' not found in group '{group_name}'")
    return scalar.text


def get_group_scalars(
    root: ET.Element, group_name: str, required: bool = True
) -> dict[str, str]:
    """Return all scalar name/value pairs within a named group.

    Args:
        root: Root element of the parsed config XML.
        group_name: Name attribute of the target <group> element.
        required: Whether a missing group is an error. Groups holding
            optional inputs are absent from some configs; those are read
            with required=False and yield an empty mapping.

    Returns:
        A mapping of scalar name to scalar text value.

    Raises:
        ValueError: If the group cannot be found and required is True.
    """
    group = root.find(f"group[@name='{group_name}']")
    if group is None:
        if not required:
            logging.info("Group not present in config, skipping: %s", group_name)
            return {}
        raise ValueError(f"Group not found in config: {group_name}")
    return {scalar.get("name", ""): scalar.text or "" for scalar in group.findall("scalar")}


def is_url(source: str) -> bool:
    """Check whether a config source string is an HTTP(S) URL.

    Args:
        source: The config_file argument as passed on the command line.

    Returns:
        True if source has an http or https scheme, False otherwise.
    """
    return urlparse(source).scheme in ("http", "https")


def resolve_raw_url(url: str) -> str:
    """Rewrite a GitHub "blob" URL to its raw content URL.

    Args:
        url: The original URL, possibly a GitHub file view URL.

    Returns:
        A URL that serves the raw file content. Non-GitHub-blob URLs
        are returned unchanged.
    """
    parsed = urlparse(url)
    if parsed.netloc == "github.com" and "/blob/" in parsed.path:
        raw_path = parsed.path.replace("/blob/", "/", 1)
        return f"https://raw.githubusercontent.com{raw_path}"
    return url


def download_from_url(url: str) -> Path:
    """Download a config file from an HTTP(S) URL to a local temp file.

    Args:
        url: HTTP(S) URL to the config file.

    Returns:
        Path to the downloaded local copy of the config file.
    """
    raw_url = resolve_raw_url(url)
    local_path = Path(tempfile.gettempdir()) / Path(urlparse(raw_url).path).name
    urllib.request.urlretrieve(raw_url, local_path)
    return local_path


def is_s3_uri(source: str) -> bool:
    """Check whether a config source string is an S3 URI.

    Args:
        source: The config_file argument as passed on the command line.

    Returns:
        True if source has an s3 scheme, False otherwise.
    """
    return urlparse(source).scheme == "s3"


def download_from_s3(uri: str) -> Path:
    """Download a config file from an S3 URI to a local temp file.

    Args:
        uri: S3 URI to the config file, e.g. s3://bucket/key/config.txt.

    Returns:
        Path to the downloaded local copy of the config file.
    """
    parsed = urlparse(uri)
    bucket = parsed.netloc
    key = parsed.path.lstrip("/")
    local_path = Path(tempfile.gettempdir()) / Path(key).name
    boto3.client("s3").download_file(bucket, key, str(local_path))
    return local_path


def resolve_config_path(source: str) -> Path:
    """Resolve the config_file argument to a concrete local XML file path.

    Accepts an HTTP(S) URL, an S3 URI, or a direct local file path. A
    directory is also accepted as a fallback, in case MAAP DPS localizes
    a "file" type input into the job's working directory and passes that
    directory (often ".") instead of the original URL or filename.

    Args:
        source: The config_file argument as passed on the command line.

    Returns:
        Path to the local config XML file to parse.

    Raises:
        FileNotFoundError: If source is a directory with no config file in it.
    """
    if is_url(source):
        return download_from_url(source)
    if is_s3_uri(source):
        return download_from_s3(source)

    path = Path(source)
    if path.is_dir():
        candidates = sorted(path.glob("*.config.txt"))
        if not candidates:
            raise FileNotFoundError(f"No *.config.txt file found in {path}")
        return candidates[0]
    return path


def resolve_log_path(log_filename: str) -> Path:
    """Resolve the log_filename argument to a concrete output log path.

    An absolute log_filename is used as given; a relative one is written
    inside OUTPUT_DIR alongside the other PGE products.

    Args:
        log_filename: The log_filename argument as passed on the command line.

    Returns:
        Path the processing log should be written to.
    """
    path = Path(log_filename)
    return path if path.is_absolute() else OUTPUT_DIR / path


def print_config_file(config_path: Path) -> str:
    """Print the config file's contents to the console.

    josfra_spdc.sh edits the config file in place before invoking the PGE,
    so this shows the config as actually processed.

    Args:
        config_path: Path to the local config XML file.

    Returns:
        The config file's contents.
    """
    contents = config_path.read_text(encoding="utf-8")
    print(f"Config file {config_path} contents:\n{contents}")
    return contents


def get_directory_listing(command: str = "ls ./*") -> subprocess.CompletedProcess[str]:
    """Run an `ls` command for debugging.

    Args:
        command: Shell command to run (defaults to `ls ./*` for the
            current working directory).

    Returns:
        The completed process, with stdout/stderr captured as text.
    """
    return subprocess.run(
        command, shell=True, capture_output=True, text=True, check=False
    )


def print_directory_listing(
    result: subprocess.CompletedProcess[str], command: str = "ls ./*"
) -> None:
    """Print a directory listing result to the console.

    Args:
        result: The completed process returned by get_directory_listing.
        command: The `ls` command that produced result, for labeling output.
    """
    print(f"{command} output:\n{result.stdout}")
    if result.stderr:
        print(f"{command} stderr:\n{result.stderr}")


def log_directory_listing(
    result: subprocess.CompletedProcess[str],
    inputs_listing: subprocess.CompletedProcess[str],
) -> None:
    """Log the working directory and /inputs/ listings.

    Args:
        result: The completed process returned by get_directory_listing.
        inputs_listing: The completed process for `ls -la /inputs/`.
    """
    logging.info("Current directory: %s", Path.cwd())

    logging.info("ls ./* output:\n%s", result.stdout)
    if result.stderr:
        logging.info("ls ./* stderr:\n%s", result.stderr)

    logging.info("ls -la /inputs/ output:\n%s", inputs_listing.stdout)
    if inputs_listing.stderr:
        logging.info("ls -la /inputs/ stderr:\n%s", inputs_listing.stderr)

    logging.info("Random string: %s", generate_random_string(RANDOM_STRING_LENGTH))


def log_executable(executable: Path, args: list[str] | None = None) -> int | None:
    """Run a bundled C++ executable and log its result.

    Logs the executable's exit status along with its stdout and stderr. If
    the executable cannot be run at all (missing, not executable, wrong
    architecture), the failure is logged instead of raising.

    Args:
        executable: Path to the executable to run.
        args: Command-line arguments to pass, or None to pass none.

    Returns:
        The executable's exit status, or None if it could not be run.
    """
    command = [str(executable), *(args or [])]
    label = " ".join(command)

    try:
        result = subprocess.run(command, capture_output=True, text=True, check=False)
    except OSError as exc:
        logging.error("Failed to run %s: %s", label, exc)
        return None

    logging.info("%s return value: %d", label, result.returncode)
    logging.info("%s stdout:\n%s", label, result.stdout)
    if result.stderr:
        logging.info("%s stderr:\n%s", label, result.stderr)
    return result.returncode


def generate_random_string(length: int) -> str:
    """Generate a random alphanumeric string of the given length.

    Args:
        length: Number of characters to generate.

    Returns:
        A random string composed of ASCII letters and digits.
    """
    return "".join(random.choices(string.ascii_letters + string.digits, k=length))


def build_output_basename(root: ET.Element, production_time: datetime) -> str:
    """Derive the shared output product basename from the config.

    The basename follows the JOSFRA naming convention:
    SNDR.AQUA.AIRS.<timestamp>.m06.g<granule>.JOSFRA.std.<version>.I.<production_timestamp>

    Args:
        root: Root element of the parsed config XML.
        production_time: Time this run produced its outputs. Passed in
            rather than read from the clock here so that the basename and
            the CAS ProductionDateTime describe the same instant.

    Returns:
        The basename shared by the .nc and .cas outputs. The processing
        log is named by the log_filename argument instead.
    """
    start_date_time = get_scalar(root, "GranuleIdentification", "StartDateTime")
    start_granule_number = get_scalar(root, "GranuleIdentification", "StartGranuleNumber")
    version = get_scalar(root, "PrimaryExecutable", "Version")

    granule_dt = datetime.strptime(start_date_time, CONFIG_TIME_FORMAT)
    timestamp = granule_dt.strftime("%Y%m%dT%H%M")
    production_timestamp = production_time.strftime(PRODUCTION_TIMESTAMP_FORMAT)

    return (
        f"SNDR.AQUA.AIRS.{timestamp}.m06.g{start_granule_number}"
        f".JOSFRA.std.{version}.I.{production_timestamp}"
    )


def write_netcdf(root: ET.Element, output_path: Path) -> None:
    """Write the NetCDF product derived from the config file.

    StartGranuleNumber, StartDateTime, Version, and a random 5000-character
    RandomString become global attributes. DynamicAuxiliaryInputFiles and
    InputProductFiles become groups, with each scalar in them stored as a
    group attribute.

    Args:
        root: Root element of the parsed config XML.
        output_path: Destination path for the .nc file.
    """
    start_date_time = get_scalar(root, "GranuleIdentification", "StartDateTime")
    start_granule_number = get_scalar(root, "GranuleIdentification", "StartGranuleNumber")
    version = get_scalar(root, "PrimaryExecutable", "Version")
    dynamic_aux_files = get_group_scalars(root, "DynamicAuxiliaryInputFiles", required=False)
    input_product_files = get_group_scalars(root, "InputProductFiles", required=False)

    with Dataset(output_path, "w", format="NETCDF4") as dataset:
        dataset.StartGranuleNumber = int(start_granule_number)
        dataset.StartDateTime = start_date_time
        dataset.Version = version
        dataset.RandomString = generate_random_string(RANDOM_STRING_LENGTH)

        aux_group = dataset.createGroup("DynamicAuxiliaryInputFiles")
        for name, value in dynamic_aux_files.items():
            setattr(aux_group, name, value)

        input_group = dataset.createGroup("InputProductFiles")
        for name, value in input_product_files.items():
            setattr(input_group, name, value)


def build_cas_metadata(
    root: ET.Element, basename: str, production_time: datetime
) -> dict[str, str]:
    """Build the CAS key/value metadata for this run's product.

    Keys and their order follow the SNDR.AQUA.JOSFRA.101.hdf.cas template.
    The granule-specific keys are filled from the same config values and
    production time that build_output_basename uses; the remaining keys
    carry the template's fixed values.

    EndDateTime is derived as StartDateTime plus the 6-minute granule
    duration rather than read from the config, whose EndDateTime is a
    millisecond short of the granule end and so would round down in the
    CAS whole-second time format.

    Args:
        root: Root element of the parsed config XML.
        basename: The shared output basename from build_output_basename.
        production_time: Time this run produced its outputs.

    Returns:
        A mapping of CAS key to value, in the order it should be written.
    """
    start_date_time = get_scalar(root, "GranuleIdentification", "StartDateTime")
    granule_number = get_scalar(root, "GranuleIdentification", "StartGranuleNumber")
    version = get_scalar(root, "PrimaryExecutable", "Version")

    granule_dt = datetime.strptime(start_date_time, CONFIG_TIME_FORMAT)
    granule_end_dt = granule_dt + timedelta(minutes=GRANULE_DURATION_MINUTES)
    product_name = f"{basename}.nc"

    return {
        "AggregateDir": "aqua_l2_josfra",
        "AlgorithmName": "JOSFRA",
        "AlgorithmProvider": "JPL",
        "AlgorithmVersion": version,
        "AutomaticQualityFlag": "Passed",
        "BuildId": "v03.21.00",
        "Collection": "production",
        "CollectionLabel": "std",
        "DataDuration": "m06",
        "DataGroup": "sndr",
        "DataProvider": "albertli",
        "DataVersion": "v02_24_00",
        "EndDateTime": granule_end_dt.strftime(CAS_TIME_FORMAT),
        "EndTAI93": "631374091.0",
        "FileFormat": "nc",
        "FileLocation": (
            "/home/albertli/deploy/pge_exe_dir/AIRSJosfra/2013-01-03_nom"
            "/2023-11-09_subm/90480011-f788-458d-9eca-e4cbf22dfd6f"
            "/cb58c0b4-192d-4fe8-8514-e0442d77572d"
        ),
        "Filename": product_name,
        "GranuleNumber": granule_number,
        "JobId": "cb58c0b4-192d-4fe8-8514-e0442d77572d",
        "Level2Type": "RET.nc",
        "ModelId": "urn:npp:AIRSJosfraNewMocca",
        "NodeInfo": "smog.jpl.nasa.gov",
        "NominalDate": granule_dt.strftime(CAS_DATE_FORMAT),
        "ProcessingLevel": "L2",
        "ProductName": product_name,
        "ProductType": "AIRS_ARCHIVED_L2",
        "ProductionDateTime": production_time.strftime(CAS_TIME_FORMAT),
        "ProductionLocation": "JPL/Caltech Sounder SIPS Integration",
        "ProductionLocationCode": "I",
        "RequestId": "222",
        "Resolution": "NA",
        "RetrievalType": "IROnly",
        "StartDateTime": granule_dt.strftime(CAS_TIME_FORMAT),
        "StartTAI93": "631373731.0",
        "SubCollection": "v02_24_00",
        "TaskId": "90480011-f788-458d-9eca-e4cbf22dfd6f",
    }


def render_cas_metadata(metadata: dict[str, str]) -> str:
    """Render CAS metadata as an OODT cas:metadata XML document.

    Args:
        metadata: Mapping of CAS key to value, in write order.

    Returns:
        The XML document text, matching the layout of the CAS template.
    """
    keyvals = [
        "<keyval type=\"vector\">\n"
        f"\t<key>{escape(key)}</key>\n"
        f"\t<val>{escape(value)}</val>\n"
        "</keyval>"
        for key, value in metadata.items()
    ]
    return (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        f'<cas:metadata xmlns:cas="{CAS_NAMESPACE}">\n\n'
        + "\n".join(keyvals)
        + "\n\n</cas:metadata>\n"
    )


def write_cas_file(
    root: ET.Element, output_path: Path, basename: str, production_time: datetime
) -> None:
    """Write the CAS metadata file for this run's product.

    Args:
        root: Root element of the parsed config XML.
        output_path: Destination path for the .cas file.
        basename: The shared output basename from build_output_basename.
        production_time: Time this run produced its outputs.
    """
    metadata = build_cas_metadata(root, basename, production_time)
    output_path.write_text(render_cas_metadata(metadata), encoding="utf-8")


def main() -> None:
    """Parse arguments, process the config file, and write PGE outputs."""
    parser = argparse.ArgumentParser(description="JOSFRA PGE for MAAP DPS")
    parser.add_argument(
        "config_file",
        help="Path, HTTP(S) URL, or s3:// URI to the PGE XML config file",
    )
    parser.add_argument(
        "log_filename",
        help="Name of the processing log file to write (relative to output/)",
    )
    args = parser.parse_args()

    directory_listing = get_directory_listing()
    print_directory_listing(directory_listing)

    inputs_listing = get_directory_listing("ls -la /inputs/")
    print_directory_listing(inputs_listing, "ls -la /inputs/")

    OUTPUT_DIR.mkdir(exist_ok=True)

    config_path = resolve_config_path(args.config_file)
    config_contents = print_config_file(config_path)
    if not config_contents.strip():
        raise SystemExit(f"Config file is empty: {config_path}")

    root = ET.parse(config_path).getroot()
    production_time = datetime.now()
    basename = build_output_basename(root, production_time)

    log_path = resolve_log_path(args.log_filename)
    log_path.parent.mkdir(parents=True, exist_ok=True)
    logging.basicConfig(
        filename=log_path,
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )

    logging.info("Starting JOSFRA PGE processing for config file: %s", args.config_file)
    logging.info("Config file %s contents:\n%s", config_path, config_contents)
    log_directory_listing(directory_listing, inputs_listing)
    log_executable(GET_BUILD_ID_PATH)
    log_executable(TAI_TO_UTC_PATH, TAI_TO_UTC_ARGS)
    if config_path != Path(args.config_file):
        logging.info("Resolved config to local file: %s", config_path)

    nc_path = OUTPUT_DIR / f"{basename}.nc"
    write_netcdf(root, nc_path)
    logging.info("Wrote NetCDF output: %s", nc_path)

    cas_path = OUTPUT_DIR / f"{basename}.cas"
    write_cas_file(root, cas_path, basename, production_time)
    logging.info("Wrote CAS output: %s", cas_path)

    logging.info("JOSFRA PGE processing complete.")

    final_directory_listing = get_directory_listing()
    print_directory_listing(final_directory_listing)
    logging.info("ls ./* output:\n%s", final_directory_listing.stdout)
    if final_directory_listing.stderr:
        logging.info("ls ./* stderr:\n%s", final_directory_listing.stderr)

    outputs_listing = get_directory_listing("ls -la /outputs/")
    print_directory_listing(outputs_listing, "ls -la /outputs/")
    logging.info("ls -la /outputs/ output:\n%s", outputs_listing.stdout)
    if outputs_listing.stderr:
        logging.info("ls -la /outputs/ stderr:\n%s", outputs_listing.stderr)


if __name__ == "__main__":
    main()
