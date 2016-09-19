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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "acquisition_sink_impl.h"
#include <iostream>

namespace gr {
  namespace gps {

    acquisition_sink::sptr
    acquisition_sink::make()
    {
      return gnuradio::get_initial_sptr
        (new acquisition_sink_impl());
    }

    /*
     * The private constructor
     */
    acquisition_sink_impl::acquisition_sink_impl()
      : gr::sync_block("acquisition_sink",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0))
    {
      message_port_register_in(pmt::mp("acq"));
      set_msg_handler(pmt::mp("acq"), boost::bind(&acquisition_sink_impl::message_handler_function, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    acquisition_sink_impl::~acquisition_sink_impl()
    {
    }

    void
    acquisition_sink_impl::message_handler_function(pmt::pmt_t msg)
    {
      std::cout << "received message" << std::endl;
      if (pmt::is_vector(msg))
      {
        // TODO we need to pass PRN numbers along if we don't explicitly try to acquire them all 
//        pmt::pmt_t sat;
        for (int i = 0; i < pmt::length(msg); i++)	// sould be 32 for now
        {
          pmt::pmt_t sat = pmt::vector_ref( msg, i );
          float acq_metric = pmt::to_float( pmt::tuple_ref( sat, 0) );
          bool is_acquired = pmt::to_bool( pmt::tuple_ref( sat, 1) );
          if (is_acquired)
            std::cout << "Acquired PRN" << i+1 << " with acquisition metric " << acq_metric << std::endl;
        }
      }
    }


    int
    acquisition_sink_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace gps */
} /* namespace gr */

