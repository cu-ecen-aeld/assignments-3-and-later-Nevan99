<<<<<<< HEAD
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
=======
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
>>>>>>> d0d984852415bc7b9f2bb2a67ec8bb385e8bfa75
