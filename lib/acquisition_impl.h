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

#ifndef INCLUDED_GPS_ACQUISITION_IMPL_H
#define INCLUDED_GPS_ACQUISITION_IMPL_H

#include "gnuradio/fft/fft.h"
#include <gps/acquisition.h>
namespace gr {
  namespace gps {

    class acquisition_impl : public acquisition {
    private:
      const int _code_freq = 1.023e6;
      const int _code_len = 1023;
      int _freqwindow, _freqbinsize, _samp_rate, _IF;
      bool _finefreqsearch;
      std::vector<std::vector<int>> PRN;

      gr::fft::fft_complex *fft = nullptr;
      gr::fft::fft_complex *ifft = nullptr;
      gr::fft::fft_complex *fftlong = nullptr;

    public:
      acquisition_impl(int, int, int, int);
      ~acquisition_impl();

      // Where all the action really happens
      int work(int noutput_items, gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);

      void generate_prn_codes(void);
      std::vector<float> pcps(std::vector<std::complex<float>>,
                              std::vector<std::complex<float>>, int, float, float);
  //  channel frequency_search(std::vector<std::complex<float> >,
  //                           std::vector<std::complex<float> >, int, int, int, int);
    };
  } // namespace gps
}   // namespace gr
#endif /* INCLUDED_GPS_ACQUISITION_IMPL_H */
