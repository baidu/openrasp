#!/bin/bash -x

flag_url=
flag_language=
flag_file=

function usage()
{
    cat << EOF
Usage: crash.sh [options...]
Options: 
 -l   Langauge
 -f   Abs path to crash file (java => hs_err_pidXXX.log)
 -u   Crash reporting url
 -h   This help text
EOF
    exit 1
}

while getopts "l:f:u:h" arg
do
    case $arg in
        l)
            flag_language=$OPTARG
            ;;
        f)
            flag_file=$OPTARG
            ;;
        u)
            flag_url=$OPTARG
            ;;
        h)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument"
            exit 1
        ;;
    esac
done

if [[ -z "$flag_language" ]] || [[ -z "$flag_file" ]] || [[ -z "$flag_url" ]]; then
    usage
fi

if [[ ! -f "$flag_file" ]]; then
    touch "$flag_file"
fi

exec curl "${flag_url}" --connect-timeout 5 \
    -F "hostname=$(hostname)" \
    -F "job=crash" \
    -F "language=$flag_language" \
    -F "crash_log=@${flag_file}"


