#!/bin/sh

set -o nounset

NUMREADER=50

rm -f /dev/shm/apitest
(./apiwr -f apitest -n ${NUMREADER}; echo $? > rv.w ) & touch rv.w
for i in `seq ${NUMREADER}`; do
    (./apird -f apitest ; echo $? > rv.$i ) & touch rv.$i ;
done

for i in `seq ${NUMREADER}`; do
    while [ ! -s rv.$i ]; do sleep 1; done ;
    if [ `cat rv.$i` -ne 0 ]; then exit 1; fi ;
done

for i in `seq ${NUMREADER}`; do rm rv.$i ; done
rm rv.w
echo "*** ${NUMREADER} api read/write test passed ***"
exit 0
