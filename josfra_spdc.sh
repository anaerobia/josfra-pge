#!/bin/bash
#
# josfra_spdc.sh -- MAAP DPS run command for the JOSFRA L2 PGE.
#
# The PGE and its environment are in the container image
# (anaerobia/josfra:<tag>). This script stages the DPS inputs, then starts
# the SPSS DPS entry point josfra_docker.sh in the image.
#
# Usage:
#   josfra_spdc.sh --config_file <f> --l1c_file <f|dir> --mocca_file <f|dir> \
#                  --forecast_file_1 <s3-uri|f> [--forecast_file_2 <s3-uri|f>] \
#                  [--forecast_file_3 <s3-uri|f>] [--forecast_file_4 <s3-uri|f>] \
#                  [--log_filename <f>]
#
# A forecast input is an S3 URI or a local file. The script downloads an S3
# URI to ./inputs with its own file name: josfra_docker.sh matches each
# forecast to the config by the valid time and forecast hour in that name.
#
# The product, the .cas file and the log go in ./output.
#
set -euo pipefail

JOSFRA_DOCKER=${JOSFRA_DOCKER:-/opt/spss/src/sips_pge/l2_josfra/scripts/josfra_docker.sh}

die() { echo "josfra-pge: $*" >&2; exit 2; }

# An unset optional CWL string arrives as a literal "" (or ''), which is
# not an empty value to the shell; strip the quotes so [ -n ] works
unquote() {
    local value="$1"
    value="${value#\"}"; value="${value%\"}"
    value="${value#\'}"; value="${value%\'}"
    printf '%s' "$value"
}

CONFIG_FILE=""
LOG_FILENAME=""
L1C_FILE=""
MOCCA_FILE=""
FORECAST_URIS=()

while [[ $# -gt 0 ]]; do
    [[ $# -ge 2 ]] || die "no value for $1"
    case $1 in
        --config_file)  CONFIG_FILE="$(unquote "$2")" ;;
        --log_filename) LOG_FILENAME="$(unquote "$2")" ;;
        --l1c_file)     L1C_FILE="$(unquote "$2")" ;;
        --mocca_file)   MOCCA_FILE="$(unquote "$2")" ;;
        --forecast_file_[1-4])
            value="$(unquote "$2")"
            if [ -n "$value" ]; then
                FORECAST_URIS+=("$value")
            fi
            ;;
        *) echo "josfra-pge: ignoring unknown argument $1" ;;
    esac
    shift 2
done

[ -n "$CONFIG_FILE" ] || die "--config_file is required"
[ -n "$L1C_FILE" ]    || die "--l1c_file is required"
[ -n "$MOCCA_FILE" ]  || die "--mocca_file is required"
[ ${#FORECAST_URIS[@]} -gt 0 ] || die "at least one --forecast_file_N is required"

# A STAC-staged input is a directory; use the first data file in it
resolve_input() {
    local label="$1" path="$2" found
    if [ -d "$path" ]; then
        found=$(find "$path" -type f \( -name '*.nc' -o -name '*.hdf' -o -name '*.h5' \) | sort | head -1)
        [ -n "$found" ] || die "$label: no .nc, .hdf or .h5 file in directory $path"
        echo "josfra-pge: $label: using $found from STAC directory" >&2
        path="$found"
    fi
    [ -f "$path" ] || die "$label not found: $path"
    realpath "$path"
}

CONFIG=$(resolve_input config_file "$CONFIG_FILE")
L1C=$(resolve_input l1c_file "$L1C_FILE")
MOCCA=$(resolve_input mocca_file "$MOCCA_FILE")

# Download each S3 forecast with its own name. A zero-byte object is a
# valid forecast input and is left alone.
mkdir -p ./inputs
FORECAST_ARGS=()
for uri in "${FORECAST_URIS[@]}"; do
    if [ -f "$uri" ]; then
        local_file="$uri"
    elif [[ "$uri" == s3://* ]]; then
        local_file="./inputs/$(basename "$uri")"
        if ! aws s3 cp --no-progress "$uri" "$local_file"; then
            die "failed to download forecast: $uri"
        fi
    else
        die "forecast is not a local file or an s3:// URI: $uri"
    fi
    FORECAST_ARGS+=(--forecast "$(realpath "$local_file")")
done

export OUTPUT_DIR="${OUTPUT_DIR:-$PWD/output}"
mkdir -p "$OUTPUT_DIR"
LOG="${LOG_FILENAME:-josfra.log}"
case "$LOG" in
    /*) ;;
    *)  LOG="$OUTPUT_DIR/$LOG" ;;
esac

echo "josfra-pge: config:   $CONFIG"
echo "josfra-pge: l1c:      $L1C"
echo "josfra-pge: mocca:    $MOCCA"
echo "josfra-pge: log:      $LOG"

set +e
"$JOSFRA_DOCKER" --pge_config "$CONFIG" \
                 --primary_input "$L1C" \
                 --secondary_input "$MOCCA" \
                 "${FORECAST_ARGS[@]}" 2>&1 | tee "$LOG"
rc=${PIPESTATUS[0]}
set -e
exit "$rc"
