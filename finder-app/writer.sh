#!/bin/bash

writefile=$1
writestr=$2

if [ $# -ne 2 ]; then
    echo "ERROR: expected 2 arguments: <writefile> <writestr>"
    exit 1
fi

mkdir -p "$(dirname "$writefile")"
echo "$writestr" > "$writefile"

if [ $? -ne 0 ]; then
    echo "ERROR: could not create file $writefile"
    exit 1
fi