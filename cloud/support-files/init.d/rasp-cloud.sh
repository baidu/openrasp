#!/bin/bash

export RASP_HOME=/home/opt/rasp-cloud

case "$1" in
    start)
        $RASP_HOME/rasp-cloud -d
    ;;

    stop)
        $RASP_HOME/rasp-cloud -s stop
    ;;

    status)
        $RASP_HOME/rasp-cloud -s status
    ;;

    restart)
        $0 stop
        $0 start
    ;;

    *)
        echo $0 start/stop/restart/status
        exit 0
    ;;
esac

