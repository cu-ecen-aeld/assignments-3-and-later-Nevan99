#!/bin/bash
<<<<<<< HEAD

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
=======
if [ $# -ne 2 ]; then
    echo "Error: Two parameters required"
    echo "Usage: $0 <writefile> <writestr>"
    exit 1
fi

writefile="$1"
writestr="$2"

writedir=$(dirname "$writefile")

if [ ! -d "$writedir" ]; then
    mkdir -p "$writedir"
    if [ $? -ne 0 ]; then
        echo "Error: Could not create directory $writedir"
        exit 1
    fi
fi

echo "$writestr" > "$writefile"
if [ $? -ne 0 ]; then
    echo "Error: Could not create file $writefile"
>>>>>>> d0d984852415bc7b9f2bb2a67ec8bb385e8bfa75
    exit 1
fi