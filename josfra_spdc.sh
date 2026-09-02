#!/bin/bash
set -ex

# Parse named args — all come in as strings
while [[ $# -gt 0 ]]; do
    case $1 in
        --pge_config)      PGE_CONFIG="$2";      shift 2 ;;
        --primary_input)   PRIMARY_INPUT="$2";   shift 2 ;;
        --secondary_input) SECONDARY_INPUT="$2"; shift 2 ;;
        --log_file)        LOG_FILE="$2";        shift 2 ;;
        --config_file)     CONFIG_FILE="$2";     shift 2 ;;
        --forecast_3hr)    FORECAST_3HR="$2";    shift 2 ;;
        --forecast_6hr)    FORECAST_6HR="$2";    shift 2 ;;
        --forecast_9hr)    FORECAST_9HR="$2";    shift 2 ;;
        *)                 shift ;;
    esac
done

# Create /inputs/ manually since DPS won't do it
mkdir -p /inputs

# DPS/CWL already localizes File inputs to a staged local path -- copy them in
cp "$PRIMARY_INPUT"   /inputs/primary_input
cp "$SECONDARY_INPUT" /inputs/secondary_input
cp "$LOG_FILE"        /inputs/log_file
cp "$CONFIG_FILE"     /inputs/config_file

# Copy optional forecast files only if provided
if [ -n "$FORECAST_3HR" ]; then
    cp "$FORECAST_3HR" /inputs/forecast_3hr
fi
if [ -n "$FORECAST_6HR" ]; then
    cp "$FORECAST_6HR" /inputs/forecast_6hr
fi
if [ -n "$FORECAST_9HR" ]; then
    cp "$FORECAST_9HR" /inputs/forecast_9hr
fi

echo "Contents of /inputs/:"
ls -la /inputs/

# Run the PGE — pge_config is also a staged local path; josfra_pge.py's
# resolve_config_path handles local paths (as well as S3/HTTP URIs) directly
python /app/josfra-pge/josfra_pge.py "$PGE_CONFIG"
