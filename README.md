# josfra-pge

MAAP DPS algorithm for the AIRS L2 JOSFRA PGE (SPSS v08.00.16).

The PGE, its Python environment and its static files are in the container
image `anaerobia/josfra:<tag>`. This repository has the MAAP run and build
commands, and a Dockerfile that adds this repository to that image at
`/app/josfra-pge`. MAAP uses the result, `anaerobia/josfra:<tag>-maap`.

| SPSS | Base image | MAAP image | Registration |
|---|---|---|---|
| v08.00.16 (branch `SIPS-2922/MOCCA-JOSFRA-Docker`, 2026-11-08) | `anaerobia/josfra:spss261108` | `anaerobia/josfra:spss261108-maap` | `algorithm.json` (`josfra-spss261108`) |
| v08.00.16 | `anaerobia/josfra:v080016` | `anaerobia/josfra:v080016-maap` | `josfra-v080016` (earlier image; does not run in DPS) |

Build the `-maap` image on `sleet2`. First, build and test the base image
there (RUNBOOK steps 7.2 to 7.5):

```bash
docker build --platform linux/amd64 --build-arg JOSFRA_TAG=spss261108 \
    -t anaerobia/josfra:spss261108-maap .
docker push anaerobia/josfra:spss261108-maap
```

The full release procedure (build, tests, the two pushes, registration and
the first job) is section 7 of `src/sips_pge/l2_josfra/docker/RUNBOOK.md` in
`SIPS/JOSFRA-v080016`. **After each change to the base image, rebuild and
push the `-maap` image, then register again.** If you do not, MAAP runs the
old code.

## MAAP registration

| Field | Value |
|---|---|
| Repository URL | `https://github.com/anaerobia/josfra-pge` |
| Run Command | `josfra-pge/josfra_spdc.sh` |
| Build Command | `josfra-pge/build-env.sh` |
| Container | `docker.io/anaerobia/josfra:spss261108-maap` |

The MAAP image makes `josfra_spdc.sh` the container ENTRYPOINT. DPS starts
a job as `docker run <image> josfra-pge/josfra_spdc.sh <args>`. With the base
image's ENTRYPOINT (`run_josfra.sh`), the run command was only an argument,
and the job stopped with `Config file not found`. The wrapper ignores its own
name when it is the first argument.

To register, send `algorithm.json` to the MAAP build API from a MAAP
Jupyter notebook:

```python
import requests
from maap.maap import MAAP
maap = MAAP()
algo = requests.get("https://raw.githubusercontent.com/anaerobia/josfra-pge/main/algorithm.json").text
r = requests.post(maap.config.maap_api_root + "build",
                  headers=maap._get_api_header(content_type="application/json"),
                  data=algo)
print(r.status_code, r.text)
```

## Inputs

| Input | Required | Description |
|---|---|---|
| `config_file` | yes | JOSFRA PGE config (XML) made by SPDC |
| `l1c_file` | yes | AIRS L1C granule. A STAC item (DPS gives a directory) or a file. For a directory, the wrapper uses the first `.nc`, `.hdf` or `.h5` file in it. |
| `mocca_file` | yes | AIRS MOCCA L2 granule (`.nc`) |
| `forecast_file_1` | yes | NCEP forecast file: an `s3://` URI or a local file |
| `forecast_file_2` to `forecast_file_4` | no | More NCEP forecast files. Use `""` for no file. |
| `log_filename` | no | Log file name. A relative name goes in `./output`. The default is `josfra.log`. |

The script downloads each `s3://` forecast to `./inputs` with its own file
name. The SPSS script `josfra_docker.sh` then sets these fields in a copy of
the config:

- `AirsL1cFile` and `AirsMoccaFile`: the `l1c_file` and `mocca_file` paths.
- The `AVN_*` forecast fields: the forecast files. Each field gets the file
  with the same valid time and forecast hour. The script reads them from the
  field name (for example `AVN_HDF_1_DEG_T06Z_F06`) and from the file name
  (for example `NCEP.2014.06.01.T12Z.gblav_PHdfF06...`).
- `SFIFFilename`: the SFIF in the image.

If the config has a forecast field and no input forecast agrees with it,
the script stops with an error.

## Outputs

`./output` contains the product, its `.cas` metadata file, the log and the
STAC metadata:

```
output/<ProductName>                                              # product
output/<ProductName>.cas
output/josfra.log
output/catalog.json
output/<ProductName without .nc>/<ProductName without .nc>.json   # STAC Item
```

The PGE writes a temporary name, for example `SNDR.AQUA.JOSFRA.101.nc`. On
a SIPS host, SPDC archives the product under the `ProductName` in its
`.cas` file. In DPS, `josfra_spdc.sh` renames the product and its `.cas`
file to `ProductName` after the PGE run. If the `.cas` file has no
`ProductName`, the file keeps its name. If a file with the new name exists,
the job stops.

After the rename, `josfra_spdc.sh` runs `make_stac.py` with
`/opt/stac-env/bin/python` (pystac 1.10.1, STAC 1.0.0). It writes the files
before DPS copies `./output` to S3. `make_stac.py` is a copy of the script
in `github.com/anaerobia/mocca-pge`, changed for JOSFRA.

- The item id is `ProductName` from the `.cas` file, without `.nc`.
- The start and end times, `BuildId` and `ProcessingLevel` come from the
  `.cas` file.
- The footprint comes from the `geospatial_*` attributes, or from the `lat`
  and `lon` variables in the product. A granule that crosses the
  antimeridian gets a MultiPolygon. A granule that reaches latitude 90 or
  -90 contains a pole, so it gets longitudes -180 to 180.
- The `.cas` fields in `EXTRA_KEYS` are copied as `josfra:<key>`, for example
  `josfra:GranuleNumber` and `josfra:SubCollection`. A field that is not in
  the `.cas` file is skipped. `FileLocation`, `NodeInfo` and `JobId` are not
  copied.
- The collection ID is the product's NetCDF `shortname` attribute, or
  `JOSFRA_COLLECTION` if it is set.
- Asset hrefs are relative unless `JOSFRA_ASSET_BASE` is set. DPS changes
  them to `s3://` hrefs when it registers the item.

To see every `.cas` key and NetCDF global attribute of a product:

```bash
/opt/stac-env/bin/python /app/josfra-pge/make_stac.py --dump output
```

## Source

The PGE source and the container build files are in the SIPS repository
`SIPS/JOSFRA-v080016` (`src/sips_pge/l2_josfra/docker` and
`src/sips_pge/l2_josfra/scripts/josfra_docker.sh`).
