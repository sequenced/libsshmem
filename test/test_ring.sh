#!/bin/sh

set -o nounset

NUMREADER=50

rm -f /dev/shm/ringtest
(./rwr -f ringtest -n ${NUMREADER}; echo $? > rv.w ) & touch rv.w
for i in `seq ${NUMREADER}`; do
    (./rrd -f ringtest ; echo $? > rv.$i ) & touch rv.$i ;
done

for i in `seq ${NUMREADER}`; do
    while [ ! -s rv.$i ]; do sleep 1; done ;
    if [ `cat rv.$i` -ne 0 ]; then exit 1; fi ;
done

for i in `seq ${NUMREADER}`; do  rm rv.$i ; done
rm rv.w
echo "*** ${NUMREADER} ring read/write test(s) passed ***"
exit 0
