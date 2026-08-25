"""JOSFRA PGE demo algorithm for MAAP DPS.

Reads a JOSFRA-style XML config file and produces a NetCDF product,
a CAS metadata file (scalar names only), and a processing log.
"""

from __future__ import annotations

import argparse
import logging
import xml.etree.ElementTree as ET
from datetime import datetime
from pathlib import Path

from netCDF4 import Dataset

OUTPUT_DIR = Path("output")


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

    StartGranuleNumber, StartDateTime, and Version become global attributes.
    DynamicAuxiliaryInputFiles and InputProductFiles become groups,
    with each scalar in them stored as a group attribute.

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
    parser.add_argument("config_file", type=Path, help="Path to the PGE XML config file")
    args = parser.parse_args()

    OUTPUT_DIR.mkdir(exist_ok=True)

    root = ET.parse(args.config_file).getroot()
    basename = build_output_basename(root)

    log_path = OUTPUT_DIR / f"{basename}.log"
    logging.basicConfig(
        filename=log_path,
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )

    logging.info("Starting JOSFRA PGE processing for config file: %s", args.config_file)

    nc_path = OUTPUT_DIR / f"{basename}.nc"
    write_netcdf(root, nc_path)
    logging.info("Wrote NetCDF output: %s", nc_path)

    cas_path = OUTPUT_DIR / f"{basename}.cas"
    write_cas_file(root, cas_path)
    logging.info("Wrote CAS output: %s", cas_path)

    logging.info("JOSFRA PGE processing complete.")


if __name__ == "__main__":
    main()
