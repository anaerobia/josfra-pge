FROM mas.maap-project.org/root/maap-workspaces/custom_images/maap_base:v6.0.0

# Install dependencies
RUN pip install netCDF4 boto3

# Copy algorithm code into /app/josfra-pge/
COPY . /app/josfra-pge/

# Make scripts executable
RUN chmod +x /app/josfra-pge/josfra_spdc.sh \
             /app/josfra-pge/build-env.sh
