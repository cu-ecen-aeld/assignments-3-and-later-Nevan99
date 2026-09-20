#!/bin/bash 

filesdir=$1
searchstr=$2

#validate argument count
if [ $# -ne 2 ]; then   
    echo "Error: expected 2 arguments: <filesdir> <searchstr>"
    exit 1
fi

#validate filesdir is a real directory

if [ ! -d "$filesdir" ]; then
    echo "Error: $filesdir is not a directory"
    exit 1
fi
numfiles=$(find "$filesdir" -type f | wc -l)
numlines=$(grep -r "$searchstr" "$filesdir" | wc -l)

echo "The number of files are $numfiles and the number of matching lines are $numlines"