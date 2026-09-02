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

# Download required File inputs
aws s3 cp "$PRIMARY_INPUT"   /inputs/primary_input
aws s3 cp "$SECONDARY_INPUT" /inputs/secondary_input
aws s3 cp "$LOG_FILE"        /inputs/log_file
aws s3 cp "$CONFIG_FILE"     /inputs/config_file

# Download optional forecast files only if provided
if [ -n "$FORECAST_3HR" ]; then
    aws s3 cp "$FORECAST_3HR" /inputs/forecast_3hr
fi
if [ -n "$FORECAST_6HR" ]; then
    aws s3 cp "$FORECAST_6HR" /inputs/forecast_6hr
fi
if [ -n "$FORECAST_9HR" ]; then
    aws s3 cp "$FORECAST_9HR" /inputs/forecast_9hr
fi

echo "Contents of /inputs/:"
ls -la /inputs/

# Run the PGE — pge_config is downloaded by josfra_pge.py itself via resolve_config_path
python /app/josfra-pge/josfra_pge.py "$PGE_CONFIG"
