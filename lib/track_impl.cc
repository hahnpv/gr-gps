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
#include "track_impl.h"

namespace gr {
  namespace gps {

    track::sptr
    track::make(int samp_rate, int prn, int codephase, float frequency)
    {
      return gnuradio::get_initial_sptr
        (new track_impl(samp_rate, prn, codephase, frequency));
    }

    /*
     * The private constructor
     */
    track_impl::track_impl(int samp_rate, int prn, int codephase, float frequency)
      : gr::block("track",
              gr::io_signature::make(<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)),
              gr::io_signature::make(<+MIN_OUT+>, <+MAX_OUT+>, sizeof(<+OTYPE+>))),
	d_pll(frequency, 25., 0.25, 0.7),
	d_dll(1.023e6, 2., 1., 0.7),
        d_samp_rate(samp_rate),
        d_prn(prn),
        d_codephase(codephase),
        d_frequency(frequency),
        d_is_tracking(prn != 0),
        d_offset(codephase+1)
    {
      if(d_is_tracking)
      {
	// set up CA for the PRN
      }
    }

    /*
     * Our virtual destructor.
     */
    track_impl::~track_impl()
    {
    }

    void
    track_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    track_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
/*
      const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];
      <+OTYPE+> *out = (<+OTYPE+> *) output_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
*/

      const gr_complex *in = (const gr_complex *) input_items[0];
      float *I_P = (float *) output_items[0];
      float *pseudorange = (float *) output_items[1];

      const uint64_t nread = this->nitems_read(0); //number of items read on port 0
      const int ninput_items = noutput_items*1023; // or something like this?

	std::cout << " ninput_items: " << ninput_items << std::endl;
      // TODO need to track the end of the last sample processed

      // if we don't have a satellite to track, look for a satellite assignment (get_tags_in_range)
      if(~d_is_tracking)
      {
        std::vector<gr::tag_t> tags;
        get_tags_in_range(tags, 0, nread, nread+ninput_items);  // verify
        if(tags.size() > 0)
        {
/*
	  // todo eventually you will only get one message to your port; this is just for testing
          pmt::pmt_t sat  = pmt::vector_ref( tags, 0 );
          int prn  = pmt::to_long( pmt::tuple_ref( sat, 0) );
          int cp = pmt::to_long( pmt::tuple_ref( sat, 1) );
	  int f  = pmt::to_long( pmt::tuple_ref( sat, 2) ); // TODO not sure long is correct here


	   std::cout << " prn: " << prn << " codephase " << cp << " " << " freq " << f << std::endl;

*/
          // TODO interpret tags from stream
	  // for teesting right now we will just set values in constructor
	  // if you find an acquired satellite tag set d_is_tracking = true 
	  // if you find an acquired satellite tag init CA for that PRN
	  // d_prn    = prn; (tuple 0)
	  // d_codephase = codephase; (tuple 1)
	  // d_frequency = frequency; (tuple 2)
	  // d_offset = nread + codephase + 1;
	  // d_is_tracking = true;
        }
      } else {
/*

	TODO before dumping in impl:
	2. CA vec needs construction (can move from function in acquisition to its own class)
	3. Recast variables in old code with d_ prefix
	4. Remove constants from original code
        5. Add check that chipWidth <= ninput_items
        6. NEED TO CHANGE TO GENERAL BLOCK : N:M not 1:N or M:1 because chipWidth

       *** NOTE *** this all is kind of funky because sometimes you will need (n+1) samples other times
	you will need (n-1) samples... so you might find you need one more sample than you have
	-> perhaps its better to just take as many samples as you can from the get-go, consume as many as you
	can, and have a leftover chunk when you return. Keep a uint counting your samples-from-zero, and proceed from
	that point.  IE your 'offset' variable in the other code. 

*/
        // if we have a satellite to track,
        std::vector<gr_complex> signal(in, in + ninput_items); // wrong?

      // Do <+signal processing+>
/*

	d_codeSize = d_codeFreq / 2048000.;
	d_chipWidth = std::ceil( (1023. - d_remcodephase) / d_codeSize);	/// NOTE THIS CALCULATION REQUIRES DOUBLE PRECISION! ///

	std::vector< std::complex<double> > sample(_chipWidth);
	std::copy(signal.begin()+offset, signal.begin()+offset+d_chipWidth, sample.begin());

	double half_bit = 0.5;
	std::vector<std::complex<double> > earlyCode(d_chipWidth),
	  promptCode(d_chipWidth),
	  lateCode(d_chipWidth);

	for (int i = 0; i < _chipWidth; i++)		// TODO algorithm
	{
	  earlyCode[i]  = std::complex<double>( CA[ std::ceil((d_remcodephase - half_bit) + ((double)i)*d_codeSize) ], 0);
	  promptCode[i] = std::complex<double>( CA[ std::ceil( d_remcodephase             + ((double)i)*d_codeSize) ], 0);
	  lateCode[i]   = std::complex<double>( CA[ std::ceil((d_remcodephase + half_bit) + ((double)i)*d_codeSize) ], 0);
	}

	d_remcodephase += (d_chipWidth)*d_codeSize - 1023;					// Remainder of the code phase

	std::vector< double > carrier(d_chipWidth+1);						// TODO eliminate +1?; verify correctness
	iota(carrier.begin(), carrier.end(), 1.);
	std::transform(carrier.begin(), carrier.end(), carrier.begin(),  [&](double x) {return ((d_carrFreq * 2 * M_PI * (x-1)) / 2048000.) + _remcarrierphase;});

	d_remcarrierphase = std::remainder( carrier[d_chipWidth], (2. * M_PI)); // - M_PI;  /// doesnt look right.

	const std::complex<double> i(0.0,1.0);
	std::vector<std::complex<double> > baseband(d_chipWidth);
	std::complex<double> early, prompt, late;
	for (int j = 0; j < d_chipWidth; j++)
	{
	  baseband[j]   = (std::exp(i * carrier[j]) * sample[j]);
	  baseband[j]   = std::conj(baseband[j] * -i); // fancy way of swapping I and Q
	  early  += baseband[j] * earlyCode[j];
	  prompt += baseband[j] * promptCode[j];
	  late   += baseband[j] * lateCode[j];
	}

	// PLL - carrier frequency
	d_pll.discriminator(prompt);
	d_carrFreq = d_pll.nco();

	// DLL - code frequency
	d_dll.discriminator(early, late);
	d_codeFreq = d_dll.nco();

	d_I_P      = prompt.real();	 												// code output, matches 10iter
	d_blksize += _chipWidth;
	d_range    = ((double)d_codephase) +  ((double)d_blksize) - ((double)d_remcodephase)/((double)d_codeSize) + 1.; // pseudorange output

	I_P = d_I_P;
	pseudorange = d_range;		// TODO its not pseudorange yet don't call it that... 
//	return offset + d_chipWidth;
*/

      // consume _chipWidth items

      // The user should assume that the number of input items = noutput_items*decimation

      }
      // Tell runtime system how many output items we produced.
      return noutput_items;

    }

  } /* namespace gps */
} /* namespace gr */

