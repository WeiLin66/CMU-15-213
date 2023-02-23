#!/bin/bash
make clean
make DEBUG=$1
./mdriver -a -v -g -t traces/
