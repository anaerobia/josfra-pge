# MAAP DPS image for the JOSFRA L2 PGE.
#
# The base image has the PGE, its Python env and its static files. MAAP runs
# the run command /app/josfra-pge/josfra_spdc.sh, so this image adds the
# repository at that path. It also adds the AWS CLI, which josfra_spdc.sh
# uses to download the forecast files, and /opt/stac-env with pystac for
# make_stac.py.
#
#   docker build --platform linux/amd64 --build-arg JOSFRA_TAG=spss261110 \
#       -t anaerobia/josfra:spss261110-maap .
#
# JOSFRA_TAG selects the base image: spss261110 (SPSS development b739bdc36,
# built on 2026-11-10), spss261108 or v080016.
ARG JOSFRA_TAG=spss261110
FROM docker.io/anaerobia/josfra:${JOSFRA_TAG}

RUN /opt/conda/bin/pip install --no-cache-dir awscli && \
    ln -sf /opt/conda/bin/aws /usr/local/bin/aws && \
    aws --version

# make_stac.py needs pystac. Keep it out of the PGE environment: build a
# separate one from the Miniforge Python. pystac 1.10.x writes STAC 1.0.0,
# the version the MAAP STAC API serves.
RUN /opt/conda/bin/python -m venv /opt/stac-env && \
    /opt/stac-env/bin/pip install --no-cache-dir pystac==1.10.1 "h5py>=3.11,<4" && \
    /opt/stac-env/bin/python -c "import pystac, h5py; print(pystac.__version__)"

COPY . /app/josfra-pge/
RUN chmod +x /app/josfra-pge/josfra_spdc.sh /app/josfra-pge/build-env.sh \
             /app/josfra-pge/make_stac.py

# DPS runs "docker run <image> josfra-pge/josfra_spdc.sh <args>". With the
# base image's ENTRYPOINT (/usr/local/bin/run_josfra.sh) the run command is
# only an argument: run_josfra.sh takes it as a config file name and stops
# with "Config file not found". Make this repository's wrapper the
# entrypoint.
ENTRYPOINT ["/app/josfra-pge/josfra_spdc.sh"]
