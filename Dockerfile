FROM almalinux:8

# Install Python and dependencies
RUN dnf -y install python39 python39-pip \
    && dnf clean all \
    && rm -rf /var/cache/dnf
RUN pip3 install --no-cache-dir netCDF4 boto3

# Copy algorithm code into /app/josfra-pge/
COPY . /app/josfra-pge/

# Make scripts executable
RUN chmod +x /app/josfra-pge/josfra_spdc.sh \
             /app/josfra-pge/build-env.sh
