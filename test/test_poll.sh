#!/bin/sh

set -o nounset

rm -f /dev/shm/polltest1-*

./apipoll -f polltest1
