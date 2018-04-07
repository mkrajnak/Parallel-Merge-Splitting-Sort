#!/bin/bash

#pocet cisel bud zadam nebo 10 :)
if [ $# -lt 2 ];then
    echo "Error";
    exit 1;
else
    numbers=$1;
    cpus=$2;
fi;

#preklad cpp zdrojaku
mpic++ --prefix /usr/local/share/OpenMPI -o mspl merge-splitting.cpp

compile=$?;
if [[ $compile != 0 ]];then
  exit $compile;
fi;

#vyrobeni souboru s random cisly
dd if=/dev/random bs=1 count=$numbers of=numbers;

#spusteni
mpirun --prefix /usr/local/share/OpenMPI -np $cpus mspl $numbers;

#uklid
rm -f oets numbers
