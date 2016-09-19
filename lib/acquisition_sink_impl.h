/* -*- c++ -*- */
/* 
 * Copyright 2016 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_GPS_ACQUISITION_SINK_IMPL_H
#define INCLUDED_GPS_ACQUISITION_SINK_IMPL_H

#include <gps/acquisition_sink.h>

namespace gr {
  namespace gps {

    class acquisition_sink_impl : public acquisition_sink
    {
     private:
      // Nothing to declare in this block.

     public:
      acquisition_sink_impl();
      ~acquisition_sink_impl();

      void message_handler_function(pmt::pmt_t message);

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace gps
} // namespace gr

#endif /* INCLUDED_GPS_ACQUISITION_SINK_IMPL_H */

