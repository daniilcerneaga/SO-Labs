#!/usr/bin/env bash
# Usage: ./report.sh access.log
if [ "$#" -ne 1 ]; then
    printf 'usage: %s <access.log>\n' "$0" >&2
    exit 1
fi
log=$1
if [ ! -f "$log" ] || [ ! -r "$log" ]; then
    printf 'no such readable file: %s\n' "$log" >&2
    exit 1
fi

total=$(wc -l < "$log")
fails=$(awk '$4=="FAIL" {n++} END {print n+0}' "$log")
printf '%s requests, %s failed\n' "$total" "$fails"
for i in {0..6}; do
    user="user$i"
    count=$(awk -v user="$user" '$3==user && $4=="FAIL" {n++} END {print n+0}' "$log")
    printf '%s: %s failures\n' "$user" "$count"
done
