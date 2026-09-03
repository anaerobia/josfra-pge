"""JOSFRA PGE demo algorithm for MAAP DPS.

Reads a JOSFRA-style XML config file and produces a NetCDF product,
a CAS metadata file (scalar names only), and a processing log.
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
from datetime import datetime
from pathlib import Path
from urllib.parse import urlparse

import boto3
from netCDF4 import Dataset

OUTPUT_DIR = Path("output")
RANDOM_STRING_LENGTH = 5000


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


def get_group_scalars(root: ET.Element, group_name: str) -> dict[str, str]:
    """Return all scalar name/value pairs within a named group.

    Args:
        root: Root element of the parsed config XML.
        group_name: Name attribute of the target <group> element.

    Returns:
        A mapping of scalar name to scalar text value.

    Raises:
        ValueError: If the group cannot be found.
    """
    group = root.find(f"group[@name='{group_name}']")
    if group is None:
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
    config_path: Path,
) -> None:
    """Log the working directory, /inputs/, and the config file used.

    Args:
        result: The completed process returned by get_directory_listing.
        inputs_listing: The completed process for `ls -la /inputs/`.
        config_path: The resolved local config file that was parsed.
    """
    logging.info("Current directory: %s", Path.cwd())

    logging.info("ls ./* output:\n%s", result.stdout)
    if result.stderr:
        logging.info("ls ./* stderr:\n%s", result.stderr)

    logging.info("ls -la /inputs/ output:\n%s", inputs_listing.stdout)
    if inputs_listing.stderr:
        logging.info("ls -la /inputs/ stderr:\n%s", inputs_listing.stderr)

    logging.info("Found config.txt file: %s", config_path.resolve())
    logging.info(
        "Found %s:\n---------------\n%s\n---------------",
        config_path,
        config_path.read_text(encoding="utf-8"),
    )

    logging.info("Random string: %s", generate_random_string(RANDOM_STRING_LENGTH))


def generate_random_string(length: int) -> str:
    """Generate a random alphanumeric string of the given length.

    Args:
        length: Number of characters to generate.

    Returns:
        A random string composed of ASCII letters and digits.
    """
    return "".join(random.choices(string.ascii_letters + string.digits, k=length))


def build_output_basename(root: ET.Element) -> str:
    """Derive the shared output product basename from the config.

    The basename follows the JOSFRA naming convention:
    SNDR.AQUA.AIRS.<timestamp>.m06.g<granule>.JOSFRA.std.<version>.I.<production_timestamp>

    Args:
        root: Root element of the parsed config XML.

    Returns:
        The basename shared by the .nc, .log, and .cas outputs.
    """
    start_date_time = get_scalar(root, "GranuleIdentification", "StartDateTime")
    start_granule_number = get_scalar(root, "GranuleIdentification", "StartGranuleNumber")
    version = get_scalar(root, "PrimaryExecutable", "Version")

    granule_dt = datetime.strptime(start_date_time, "%Y-%m-%dT%H:%M:%S.%fZ")
    timestamp = granule_dt.strftime("%Y%m%dT%H%M")
    production_timestamp = datetime.now().strftime("%y%m%d%H%M%S")

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
    dynamic_aux_files = get_group_scalars(root, "DynamicAuxiliaryInputFiles")
    input_product_files = get_group_scalars(root, "InputProductFiles")

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


def write_cas_file(root: ET.Element, output_path: Path) -> None:
    """Write the CAS file listing every scalar name found in the config.

    Args:
        root: Root element of the parsed config XML.
        output_path: Destination path for the .cas file.
    """
    scalar_names = [scalar.get("name", "") for scalar in root.iter("scalar")]
    output_path.write_text("\n".join(scalar_names) + "\n", encoding="utf-8")


def main() -> None:
    """Parse arguments, process the config file, and write PGE outputs."""
    parser = argparse.ArgumentParser(description="JOSFRA PGE for MAAP DPS")
    parser.add_argument(
        "config_file",
        help="Path, HTTP(S) URL, or s3:// URI to the PGE XML config file",
    )
    args = parser.parse_args()

    directory_listing = get_directory_listing()
    print_directory_listing(directory_listing)

    inputs_listing = get_directory_listing("ls -la /inputs/")
    print_directory_listing(inputs_listing, "ls -la /inputs/")

    OUTPUT_DIR.mkdir(exist_ok=True)

    config_path = resolve_config_path(args.config_file)

    root = ET.parse(config_path).getroot()
    basename = build_output_basename(root)

    log_path = OUTPUT_DIR / f"{basename}.log"
    logging.basicConfig(
        filename=log_path,
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )

    logging.info("Starting JOSFRA PGE processing for config file: %s", args.config_file)
    log_directory_listing(directory_listing, inputs_listing, config_path)
    if config_path != Path(args.config_file):
        logging.info("Resolved config to local file: %s", config_path)

    nc_path = OUTPUT_DIR / f"{basename}.nc"
    write_netcdf(root, nc_path)
    logging.info("Wrote NetCDF output: %s", nc_path)

    cas_path = OUTPUT_DIR / f"{basename}.cas"
    write_cas_file(root, cas_path)
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
