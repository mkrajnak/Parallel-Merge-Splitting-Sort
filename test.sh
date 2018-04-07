#!/bin/bash

# Check args
if [ $# -lt 2 ];then
    echo "Usage: ./test.sh <count_of_numbers_to_sort> <cpu_count>";
    exit 1;
else
    numbers=$1;
    cpus=$2;
fi;

mpic++ --prefix /usr/local/share/OpenMPI -o mspl merge-splitting.cpp
# Check if translation was successfull to avoid to many err messages
compile=$?;
if [[ $compile != 0 ]];then
  exit $compile;
fi;

# Make "file" with random numbers, send output to black hole :O
dd if=/dev/random bs=1 count=$numbers of=numbers &> /dev/null

# Run
mpirun --prefix /usr/local/share/OpenMPI -np $cpus mspl $numbers;

# Cleanup
rm -f oets numbers
