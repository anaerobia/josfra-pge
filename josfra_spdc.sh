#!/bin/bash
set -ex

# OGC DPS passes inputs as named flags -- parse them
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

# josfra_pge.py takes pge_config as its positional config_file argument
python /app/josfra-pge/josfra_pge.py "$PGE_CONFIG"
