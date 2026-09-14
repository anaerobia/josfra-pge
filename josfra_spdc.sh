#!/bin/bash
set -ex

# Parse named arguments from OGC CWL runner
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

# Create ./inputs/ so pge_config can reference predictable paths
mkdir -p ./inputs

# Copy CWL-staged files to ./inputs/ with predictable names
[ -n "$PRIMARY_INPUT" ]   && cp "$PRIMARY_INPUT"   ./inputs/primary_input
[ -n "$SECONDARY_INPUT" ] && cp "$SECONDARY_INPUT" ./inputs/secondary_input
[ -n "$LOG_FILE" ]        && cp "$LOG_FILE"        ./inputs/log_file
[ -n "$CONFIG_FILE" ]     && cp "$CONFIG_FILE"     ./inputs/config_file
[ -n "$FORECAST_3HR" ]    && cp "$FORECAST_3HR"    ./inputs/forecast_3hr
[ -n "$FORECAST_6HR" ]    && cp "$FORECAST_6HR"    ./inputs/forecast_6hr
[ -n "$FORECAST_9HR" ]    && cp "$FORECAST_9HR"    ./inputs/forecast_9hr

echo "Contents of ./inputs/:"
ls -la ./inputs/

# Run PGE — pge_config is an S3 URI, resolve_config_path downloads it
python /app/josfra-pge/josfra_pge.py "$PGE_CONFIG"