#pragma once
class filter
{
public:
	filter(double frequency, double bw, double k, double zeta) :
		_freq(frequency),
		_nco(0.),
		_last_err(0.),
		_err(0.)
	{
		// calc filter coefficients
		double wn = (8*zeta*bw)/(4*zeta*zeta+1);
		_tau1 = k/(wn*wn);
		_tau2 = 2.*zeta/wn;
	}
	void discriminator(){}

	double nco() // numerically controlled oscillator
	{
		_nco += (_tau2/_tau1) * (_err - _last_err) + _err * (0.001/_tau1);
		_last_err = _err;
		return _freq + _nco;
	}

	double frequency() {	return _freq;}
	double get_nco() 	  {	return _nco;}
	double get_error()     {	return _err;}

protected:
	double _freq;
	double _tau1, _tau2;
	double _nco;
	double _err, _last_err;
};


class pll : public filter
{
public:
	pll(double frequency, double bw, double k, double zeta)
	: filter(frequency, bw, k, zeta)
	{}
	void discriminator(std::complex<double> prompt)
	{
		// tbl 7.1 eqn 3 normalized by 2pi
		_err = atan(prompt.imag() / prompt.real()) / (2.*M_PI);
	}
};

class dll : public filter
{
public:
	dll(double frequency, double bw, double k, double zeta)
	: filter(frequency, bw, k, zeta)
	{}
	void discriminator(std::complex<double> early, std::complex<double> late)
	{
		// tbl 7.2 noncoherent #2 - TODO squrts not in table! but are in code.
		_err = (std::sqrt(early.real()*early.real() + early.imag()*early.imag()) - std::sqrt(late.real()*late.real() + late.imag()*late.imag())) /
			   (std::sqrt(early.real()*early.real() + early.imag()*early.imag()) + std::sqrt(late.real()*late.real() + late.imag()*late.imag()));
	}

	double nco() // numerically controlled oscillator
	{
		_nco += (_tau2/_tau1) * (_err - _last_err) + _err * (0.001/_tau1);
		_last_err = _err;
		return _freq - _nco;
	}

};
