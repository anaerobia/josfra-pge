#!/bin/bash
set -ex

echo "SHELL PWD: $(pwd)"
echo "SHELL ls ./*:"
ls ./* 2>/dev/null || echo "(empty)"

# Parse named arguments from OGC CWL runner
while [[ $# -gt 0 ]]; do
    case $1 in
        --config_file)      CONFIG_FILE="$2";      shift 2 ;;
        --log_filename)     LOG_FILENAME="$2";     shift 2 ;;
        --l1c_file)         L1C_FILE="$2";         shift 2 ;;
        --mocca_file)       MOCCA_FILE="$2";       shift 2 ;;
        --forecast_file_1)  FORECAST_FILE_1="$2";  shift 2 ;;
        --forecast_file_2)  FORECAST_FILE_2="$2";  shift 2 ;;
        --forecast_file_3)  FORECAST_FILE_3="$2";  shift 2 ;;
        --forecast_file_4)  FORECAST_FILE_4="$2";  shift 2 ;;
        *)                  shift ;;
    esac
done

# The config file is edited in place below, so it must be a local file
if [ ! -f "$CONFIG_FILE" ]; then
    echo "config_file is not a local file: $CONFIG_FILE" >&2
    exit 1
fi

# Rewrite a <scalar name="..."> value in the config file, in place
set_config_scalar() {
    local name="$1" value="$2" escaped
    escaped=$(printf '%s' "$value" | sed -e 's/[\\&|]/\\&/g')
    sed -i -E "s|(<scalar name=\"${name}\">)[^<]*(</scalar>)|\1${escaped}\2|" "$CONFIG_FILE"
}

# Point the input product scalars at the CWL-staged files
if [ -n "$L1C_FILE" ]; then
    set_config_scalar AirsL1cFile "$(realpath "$L1C_FILE")"
fi
if [ -n "$MOCCA_FILE" ]; then
    set_config_scalar AirsMoccaFile "$(realpath "$MOCCA_FILE")"
fi

# Run PGE
python /app/josfra-pge/josfra_pge.py "$CONFIG_FILE" "$LOG_FILENAME"
