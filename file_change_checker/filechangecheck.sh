#!/bin/bash
usage="$(basename "$0") [filepath | -h] [period_lenth_sec] -- program to periodically check if a file has changed or not; in case no change happened during the period the system will be restarted

where:
    filepath            path to the file to check
    -h                  show this help text
    period_lenth_sec    set the length of periods in seconds (default: 120)"

period_sec=120
if [ $# -ne 0 ]; then
    if [ "$1" == "-h" ]; then
        echo "Usage: ${usage}"
        exit 0
    elif [ $# -gt 1 ]; then
        period_sec=$2
    fi
elif [ $# -eq 0 ]; then
    echo "Usage: ${usage}"
    exit 0
fi

timestamp=$(stat -c %Y $1)
while true; do
    sleep $period_sec
    new_timestamp=$(stat -c %Y $1)
    if [ $timestamp -eq $new_timestamp ]; then
        reboot
    else
        timestamp=$new_timestamp
    fi
done
