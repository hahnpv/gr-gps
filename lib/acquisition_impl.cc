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
#include "acquisition_impl.h"
#include "gnuradio/fft/fft.h"
#include <iostream>   // temporarily
#include <vector>     // until we VOLK
#include <algorithm>  // until we VOLK

namespace gr {
  namespace gps {

    acquisition::sptr
    acquisition::make(int samp_rate, int IF, int freq_window, int freq_bin_size)
    {
      return gnuradio::get_initial_sptr
        (new acquisition_impl(samp_rate, IF, freq_window, freq_bin_size));
    }

    /*
     * The private constructor
     */
    acquisition_impl::acquisition_impl(int samp_rate, int IF, int freq_window, int freq_bin_size)
      : gr::sync_block("acquisition",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        _samp_rate(samp_rate),
        _IF(IF),
        _freqwindow(freq_window),
        _freqbinsize(freq_bin_size),
        PRN(33),
        _finefreqsearch(false)
    {
	set_output_multiple(_samp_rate * 0.11);
	set_min_output_buffer(_samp_rate * 0.11);
        generate_prn_codes();
    }

    /*
     * Our virtual destructor.
     */
    acquisition_impl::~acquisition_impl()
    {
    }

    int
    acquisition_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      std::vector< gr_complex > signal(in, in+noutput_items);


        // Do <+signal processing+>
	int samples_per_code =  _samp_rate / ( _code_freq / _code_len);

	// acquisition
	std::vector<std::complex<float> > sample(signal.begin(), signal.begin()+samples_per_code*10);
	std::vector<std::complex<float> >  samp0(signal.begin(), signal.begin()+samples_per_code);
	std::vector<std::complex<float> >  samp1(signal.begin()+samples_per_code, signal.begin()+2*samples_per_code);

        std::cout << "loop" << std::endl;
	for (int prn = 1; prn < 33; prn++)	// cliff's array aligns with PRN not zero-count
	{
			std::vector<int> CA(_code_len); // TODO lambda
			std::transform(PRN[prn].begin(), PRN[prn].end(), CA.begin(), [](int(x)){ return (x > 0) ? 1 : -1; } );


			std::vector<std::complex<float> > codeSamples(samples_per_code);
			for (int i = 1; i <= samples_per_code; i++)		// TODO algorithm
				codeSamples[i-1] = std::complex<float>( CA[ floor( ((float)i-1)/((float)_samp_rate/(float)_code_freq))], 0); // NOTE: This impl fixed matalb off-by-1 error but lowers prn22 in testing from 2.06 to 1.999 so threshold needs to drop

			////////////////////////////////////////////////////////////////////////////////
			if (fft==nullptr)
				fft = new gr::fft::fft_complex (samples_per_code, true, 1);
			memcpy(fft->get_inbuf(), &codeSamples[0], samples_per_code*sizeof(std::complex<float>));
			fft->execute();
			std::vector< std::complex<float> > fftCA(fft->get_outbuf(), fft->get_outbuf()+samples_per_code);
		    ////////////////////////////////////////////////////////////////////////////////

			std::transform(fftCA.begin(), fftCA.end(), fftCA.begin(), [](std::complex<float> const& x) { return std::conj(x); } );

			std::vector< std::vector<float> > acq_coarse; // TODO results are real can likely just cast to float
			for (int f = 0; f <= (_freqwindow / _freqbinsize) ; f++)
			{
				float searchfreq = _IF - _freqwindow/2 + f*_freqbinsize;
				float         dt = (2*M_PI/_samp_rate);

				std::vector< float > convIQ0Re = pcps(samp0, fftCA, samples_per_code, searchfreq, dt);
				std::vector< float > convIQ1Re = pcps(samp1, fftCA, samples_per_code, searchfreq, dt);

				// TODO refactor: we don't need to save all this data just need to save peak statistics and code phase, unless you want plots
				float zero = *std::max_element(convIQ0Re.begin(), convIQ0Re.end());
				float one  = *std::max_element(convIQ1Re.begin(), convIQ1Re.end());
				if (zero > one)
					acq_coarse.push_back(convIQ0Re);
				else
					acq_coarse.push_back(convIQ1Re);
			}

			// Find strongest correlation
			float value = 0;
			int binnum   = 0;
			int codephase  = 0;
			for (unsigned int i = 0; i< acq_coarse.size() ; i++ )
			{
				std::vector<float>::iterator result;
				result = std::max_element(acq_coarse[i].begin(), acq_coarse[i].end());
				if (*result > value)
				{
					value   = *result;
					binnum  = i;
					codephase = std::distance(acq_coarse[i].begin(), result);
				}
				//			std::cout << "value: " << value << " binnum: " << binnum << " binitem: " << binitem << std::endl;
			}

			int window = (float)_samp_rate/(float)_code_freq;
			std::vector<float> bin = acq_coarse[binnum];
			std::fill(bin.begin()+codephase-window, bin.begin()+codephase+window+1, 0);		// TODO not necessarily safe

			float peak2 = *std::max_element(bin.begin(), bin.end());

			if (value / peak2 > 1.99) // TODO we should look at C_No
			{
				if(_finefreqsearch)
				{
//					channel c = frequency_search(codeSamples,sample,prn,binnum,codephase,samples_per_code);
//					retval.push_back( c );				// TODO check signs
				} else {
//					retval.push_back( channel{prn, codephase, _IF - _freqwindow/2 + binnum*_freqbinsize, _freqwindow/2 + binnum*_freqbinsize } );				// TODO check signs
					std::cout << prn << "\t" << codephase << "\t" << binnum << "\t" << _IF - _freqwindow/2 + binnum*_freqbinsize << "\t" << value/peak2 << std::endl;
				}
			}
	}
      // TODO create messages from retval


      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

	void
        acquisition_impl::generate_prn_codes(void)
	{
		int i,j,G1,G2,prn,chip, half_chip;
		int G2_i[33] = {
				0x000, 0x3f6, 0x3ec, 0x3d8, 0x3b0, 0x04b, 0x096, 0x2cb, 0x196,
				0x32c, 0x3ba, 0x374, 0x1d0, 0x3a0, 0x340, 0x280, 0x100,
				0x113, 0x226, 0x04c, 0x098, 0x130, 0x260, 0x267, 0x338,
				0x270, 0x0e0, 0x1c0, 0x380, 0x22b, 0x056, 0x0ac, 0x158};

		for (prn=1;prn<33;prn++)
		{
			char prn_code[1023];
			prn_code[0]=1;
			PRN[prn].push_back(1);
			G1 = 0x1FF;
			G2 = G2_i[prn];
			for (chip=1;chip<1023;chip++)
			{
				prn_code[chip]=(G1^G2) & 0x1;  // exor the right hand most bit
				PRN[prn].push_back(   prn_code[chip]);
				i=((G1<<2)^(G1<<9)) & 0x200;
				G1=(G1>>1) | i;
				j=((G2<<1)^(G2<<2)^(G2<<5)^(G2<<7)^(G2<<8)^(G2<<9)) & 0x200;
				G2=(G2>>1) | j;
			}
		}
	}

	// Parallel Code Phase Search
	std::vector<float>
        acquisition_impl::pcps(std::vector<std::complex<float> > samp, std::vector<std::complex<float> > fftCA, int samples_per_code, float searchfreq, float dt)
	{
		std::vector< std::complex<float> > carrier(samples_per_code);
		const std::complex<float> i(0.0,1.0);
		iota(carrier.begin(), carrier.end(), 1.);
		std::transform(carrier.begin(), carrier.end(), carrier.begin(),  [&](std::complex< float> x) {
			return std::exp( dt * searchfreq * x * i );
		});

		// TODO can do iq0/iq2 in loop instead of repeating every command?
		std::vector< std::complex<float> > IQ(samples_per_code);
		std::transform( carrier.begin(), carrier.end(), samp.begin(), IQ.begin(), std::multiplies<std::complex<float> >() );

		////////////////////////////////////////////////////////////////////////////////
	    memcpy(fft->get_inbuf(), &IQ[0], samples_per_code*sizeof(std::complex<float>));
	    fft->execute();
	    std::vector< std::complex<float> > fftIQ(fft->get_outbuf(), fft->get_outbuf()+samples_per_code);
	    ////////////////////////////////////////////////////////////////////////////////

		std::transform( fftIQ.begin(), fftIQ.end(), fftCA.begin(), fftIQ.begin(), std::multiplies<std::complex<float> >() );

		////////////////////////////////////////////////////////////////////////////////
		if (ifft==nullptr)
			ifft = new gr::fft::fft_complex (samples_per_code, false, 1);
		memcpy(ifft->get_inbuf(), &fftIQ[0], samples_per_code*sizeof(std::complex<float>));
	    ifft->execute();
	    std::vector< std::complex<float> > convIQ(ifft->get_outbuf(), ifft->get_outbuf()+samples_per_code);
	    ////////////////////////////////////////////////////////////////////////////////

		std::vector< float > convIQRe(samples_per_code);

		std::transform(convIQ.begin(), convIQ.end(), convIQRe.begin(),  [](std::complex< float> x) { return std::pow( std::abs( x ), 2); } );

		// todo delete for each new except retval
		return convIQRe;

	}
/*
	// Fine frequency search
	channel
	acquisition_impl::frequency_search(std::vector<std::complex<float> > codeSamples, std::vector<std::complex<float> > sample, int prn, int binnum, int codephase, int samples_per_code)
	{
		//				std::cout << "Found a satellite that exceeds threshold: PRN" << prn << " " << value/peak2 << std::endl;
		// generate 10ms CA code
		std::vector< std::complex<float> > longCodeSamples = codeSamples;		// CA

		for (int i = 0; i < 4; i++)
			longCodeSamples.insert(longCodeSamples.end(), codeSamples.begin(), codeSamples.end());	// CA
		longCodeSamples.resize(samples_per_code*10);

		std::vector< std::complex<float> > longCarrier(samples_per_code*10);
		std::transform( longCodeSamples.begin(), longCodeSamples.end(), sample.begin()+codephase, longCarrier.begin(), std::multiplies<std::complex<float> >() );

		int fftsize = 8*std::pow(2, std::ceil(std::log(samples_per_code*10)/std::log(2)));
		longCarrier.resize(fftsize, std::complex<float>(0, 0));

		////////////////////////////////////////////////////////////////////////////////
		if (fftlong==nullptr)
			fftlong = new gr::fft::fft_complex (fftsize, true, 1);
		memcpy(fftlong->get_inbuf(), &longCarrier[0], fftsize*sizeof(std::complex<float>));
		fftlong->execute();
		std::vector< std::complex<float> > fftCarrier(fftlong->get_outbuf(), fftlong->get_outbuf()+fftsize);
	    ////////////////////////////////////////////////////////////////////////////////


		int peakindx = std::max_element(fftCarrier.begin(), fftCarrier.end(), [](std::complex<float> x, std::complex<float> y){
			return std::abs( x ) < std::abs( y );
		}) - fftCarrier.begin();

		int fft_unique = fftsize / 2 + 1;
		float df = ((float)_samp_rate) / ((float)fftsize);
		assert( fftsize%2 == 0);

		std::cout << prn << "\t" << peakindx << "\t";

		//WAS				float carrierFreq = -df * (peakindx-1);
		float carrierFreq = -df * (peakindx-1);

		if (peakindx > fft_unique)
		{
			peakindx = fftsize - peakindx;
			carrierFreq = df * (peakindx-1);
			//		std::cout << "SIGN FLIP" << std::endl;
		}

		std::cout << peakindx << "\t" << codephase << "\t" << binnum << "\t" << carrierFreq << std::endl; //  "\t" << carrierFreq - IF << "\t" << -df * (peakindx -1) << std::endl;
		// coarse bin matches (-1, c++)
		// TODO codephase is off by 2. Off by 1 makes sense (matlab->c++)

		return channel{prn, codephase, carrierFreq, carrierFreq - IF };				// TODO check signs

		// TODO need to save offset/codephase for tracking, peak metric for plotting
		/// OFFSETS ARE CLOSE!!! WITHIN A FEW Hz ON VALID SATELLITES. MAY BE ROUNDING ERROR. REFACTOR FOR CLARITY THEN DIG IN

	}
*/

  } /* namespace gps */
} /* namespace gr */

