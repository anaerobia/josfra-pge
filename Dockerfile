# MAAP DPS image for the JOSFRA L2 PGE.
#
# The base image has the PGE, its Python env and its static files. MAAP runs
# the run command /app/josfra-pge/josfra_spdc.sh, so this image adds the
# repository at that path. It also adds the AWS CLI, which josfra_spdc.sh
# uses to download the forecast files.
#
#   docker build --platform linux/amd64 --build-arg JOSFRA_TAG=v080016 \
#       -t anaerobia/josfra:v080016-maap .
#
# JOSFRA_TAG selects the base image: v080016 (SPSS v08.00.16).
ARG JOSFRA_TAG=v080016
FROM docker.io/anaerobia/josfra:${JOSFRA_TAG}

RUN /opt/conda/bin/pip install --no-cache-dir awscli && \
    ln -sf /opt/conda/bin/aws /usr/local/bin/aws && \
    aws --version

COPY . /app/josfra-pge/
RUN chmod +x /app/josfra-pge/josfra_spdc.sh /app/josfra-pge/build-env.sh
