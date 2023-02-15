#!/bin/bash
make clean
make
./mdriver -a -v -g -t traces/
