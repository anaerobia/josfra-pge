#!/bin/bash
#
# build-env.sh -- MAAP DPS build command for the JOSFRA L2 PGE.
#
# The container image (anaerobia/josfra:<tag>-maap) has the PGE, its Python
# env, the static files and the AWS CLI. Nothing is installed here. The
# script only makes sure that the image is correct.
#
set -euo pipefail

test -x /opt/spss/src/sips_pge/l2_josfra/scripts/josfra_docker.sh
test -x /opt/spss/src/pge_wrapper/utils/bin/getBuildId
/opt/spss/src/pge_wrapper/utils/bin/getBuildId ''
aws --version
echo "Build environment check complete."
