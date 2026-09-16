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

# Show what the runner actually handed us for config_file
echo "config_file: $CONFIG_FILE"
ls -la "$CONFIG_FILE" || true

# The config file is copied and edited below, so it must be a local file
if [ ! -f "$CONFIG_FILE" ]; then
    echo "config_file is not a local file: $CONFIG_FILE" >&2
    exit 1
fi
if [ ! -s "$CONFIG_FILE" ]; then
    echo "config_file is empty: $CONFIG_FILE" >&2
    echo "Check that the CWL config_file input points at the PGE XML config." >&2
    exit 1
fi

# Copy the config to a writable local path first; the CWL-staged copy
# may be read-only
LOCAL_CONFIG=./pge_config_local.xml
cp -f "$CONFIG_FILE" "$LOCAL_CONFIG"
chmod u+w "$LOCAL_CONFIG"

# Rewrite a <scalar name="..."> value in the local config copy
set_config_scalar() {
    local name="$1" value="$2" escaped
    escaped=$(printf '%s' "$value" | sed -e 's/[\\&|]/\\&/g')
    sed -i -E "s|(<scalar name=\"${name}\">)[^<]*(</scalar>)|\1${escaped}\2|" "$LOCAL_CONFIG"
}

# Point the input product scalars at the CWL-staged files
if [ -n "$L1C_FILE" ]; then
    set_config_scalar AirsL1cFile "$(realpath "$L1C_FILE")"
fi
if [ -n "$MOCCA_FILE" ]; then
    set_config_scalar AirsMoccaFile "$(realpath "$MOCCA_FILE")"
fi

# Run PGE on the edited local config copy
python /app/josfra-pge/josfra_pge.py "$LOCAL_CONFIG" "$LOG_FILENAME"
