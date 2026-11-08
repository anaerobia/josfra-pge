# josfra-pge

MAAP DPS algorithm for the AIRS L2 JOSFRA PGE (SPSS v08.00.16).

The PGE, its Python environment and its static files are in the container
image `anaerobia/josfra:<tag>`. This repository has the MAAP run and build
commands, and a Dockerfile that adds this repository to that image at
`/app/josfra-pge`. MAAP uses the result, `anaerobia/josfra:<tag>-maap`.

| SPSS | Base image | MAAP image | Registration |
|---|---|---|---|
| v08.00.16 | `anaerobia/josfra:v080016` | `anaerobia/josfra:v080016-maap` | `algorithm.json` (`josfra-v080016`) |

Build the `-maap` image on `sleet2`. First, build and test the base image
there (RUNBOOK steps 7.2 to 7.5):

```bash
docker build --platform linux/amd64 --build-arg JOSFRA_TAG=v080016 \
    -t anaerobia/josfra:v080016-maap .
docker push anaerobia/josfra:v080016-maap
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
| Container | `docker.io/anaerobia/josfra:v080016-maap` |

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

`./output` contains the product, its `.cas` metadata file and the log.

## Source

The PGE source and the container build files are in the SIPS repository
`SIPS/JOSFRA-v080016` (`src/sips_pge/l2_josfra/docker` and
`src/sips_pge/l2_josfra/scripts/josfra_docker.sh`).
