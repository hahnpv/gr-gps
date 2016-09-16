#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/hahnpv/SDR/gr-gps/lib
export PATH=/home/hahnpv/SDR/gr-gps/build/lib:$PATH
export LD_LIBRARY_PATH=/home/hahnpv/SDR/gr-gps/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PYTHONPATH
test-gps 
