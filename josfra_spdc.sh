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

# Create ./inputs/ so config_file can reference predictable paths
mkdir -p ./inputs

# Copy CWL-staged files to ./inputs/ with predictable names
[ -n "$L1C_FILE" ]        && cp "$L1C_FILE"        ./inputs/l1c_file
[ -n "$MOCCA_FILE" ]      && cp "$MOCCA_FILE"      ./inputs/mocca_file
[ -n "$FORECAST_FILE_1" ] && cp "$FORECAST_FILE_1" ./inputs/forecast_file_1
[ -n "$FORECAST_FILE_2" ] && cp "$FORECAST_FILE_2" ./inputs/forecast_file_2
[ -n "$FORECAST_FILE_3" ] && cp "$FORECAST_FILE_3" ./inputs/forecast_file_3
[ -n "$FORECAST_FILE_4" ] && cp "$FORECAST_FILE_4" ./inputs/forecast_file_4

echo "Contents of ./inputs/:"
ls -la ./inputs/

# Run PGE — config_file is an S3 URI, resolve_config_path downloads it
python /app/josfra-pge/josfra_pge.py "$CONFIG_FILE" "$LOG_FILENAME"
