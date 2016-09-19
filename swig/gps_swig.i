/* -*- c++ -*- */

#define GPS_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "gps_swig_doc.i"

%{
#include "gps/acquisition.h"
#include "gps/acquisition_sink.h"
%}


%include "gps/acquisition.h"
GR_SWIG_BLOCK_MAGIC2(gps, acquisition);
%include "gps/acquisition_sink.h"
GR_SWIG_BLOCK_MAGIC2(gps, acquisition_sink);
