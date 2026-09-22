#!/bin/bash

if [ $# -ne 2 ]; then 
    echo "Error: Two parameters required"
    echo "Usage: $0 <filesdir> <searchstr>"
    exit 1
fi 
filedir="$1"
searchstr="$2"

if [ ! -d "$filedir" ]; then
    echo "Error: $filesdir is not a directory"
    exit 1
fi
#2

file_count=$(find "$filedir" -type f| wc -l)
matching_lines=$(grep -r "$searchstr" "$filedir" | wc -l)
echo "The number of files are $file_count and the number of matching lines are $matching_lines"
