#include <iostream>
#include <cstdio>
#include "ranvar.h"
#include <cmath>
#include <map>

#define PI 3.14159265358979323846

using namespace std;

double gamma0 = 1.9;
double gamma1 = 3.8;
double gamma2 = 3.8;
double d0_gamma = 200;
double d1_gamma = 500;
double m0 = 1.5;
double m1 = 0.75;
double m2 = 0.75;
double d0_m = 80;
double d1_m = 200;
double lambda = 0.0508475;
double L = 1.0; // system Loss
double Pt = 20.0; // em Dbm
double Gt = 1.0;
double Gr = 1.0;
double noise_floor = 1.26e-13; // em Watts

/// Round a double number to the given precision. e.g. dround(0.234, 0.1) = 0.2
/// and dround(0.257, 0.1) = 0.3
static double dround(double number, double precision) {
	number /= precision;
	if (number >= 0) {
		number = floor(number + 0.5);
	} else {
		number = ceil(number - 0.5);
	}
	number *= precision;
	return number;
}

double Friis(double Pt, double Gt, double Gr, double lambda, double L, double d) {
	/*
	 * Friis free space equation:
	 *
	 *       Pt * Gt * Gr * (lambda^2)
	 *   P = --------------------------
	 *       (4 * pi * d)^2 * L
	 */
	if (d == 0.0) //XXX probably better check < MIN_DISTANCE or some such
		return Pt;
	double M = lambda / (4 * PI * d);

	return (Pt * Gt * Gr * (M * M)) / L;
}

// distance between nodes, system loss, lambda, transmission gain, receprion gain, transmission power
double GetPr(bool use_nakagami_dist_, double dist, double Gt, double Gr,
		double Pt) {
	double d_ref = 1.0;

	// calculate receiving power at reference distance
	double Pr0 = Friis(Pt, Gt, Gr, lambda, L, d_ref);
	// calculate average power loss predicted by empirical loss model in dB
	// according to measurements, 
	// the default settings of gamma, m and d are stored in tcl/lib/ns-default.tcl
	double path_loss_dB = 0.0;

	if (dist > 0 && dist <= d0_gamma) {
		path_loss_dB = 10 * gamma0 * log10(dist / d_ref);
	}
	if (dist > d0_gamma && dist <= d1_gamma) {
		path_loss_dB = 10 * gamma0 * log10(d0_gamma / d_ref) + 10 * gamma1
				* log10(dist / d0_gamma);
	}
	if (dist > d1_gamma) {
		path_loss_dB = 10 * gamma0 * log10(d0_gamma / d_ref) + 10 * gamma1
				* log10(d1_gamma / d0_gamma) + 10 * gamma2 * log10(
				dist / d1_gamma);
	}

	// calculate the receiving power at distance dist
	double Pr = Pr0 * pow(10.0, -path_loss_dB / 10.0);

	if (!use_nakagami_dist_) {
		return Pr;
	} else {
		double m;
		if (dist <= d0_m)
			m = m0;
		else if (dist <= d1_m)
			m = m1;
		else
			m = m2;

		unsigned int int_m = (unsigned int) (floor(m));

		double resultPower;

		if (int_m == m) {
			resultPower = ErlangRandomVariable(Pr / m, int_m).value();
		} else {
			resultPower = GammaRandomVariable(m, Pr / m).value();
		}

		return resultPower;
	}
}

double mWtoW(double m_watt) {
	return m_watt / 1000.0;
}

double WtomW(double watt) {
	return 1000.0 * watt;
}

double DBmtoW(double dbm) {
	return pow(10.0, (dbm - 30.0) / 10.0);
}

double WtoDBm(double watt) {
	return 10 * log10(watt) + 30.0;
}

double BSK_SINR_ratio = 3.1632;
double QPSK_SINR_ratio = 6.3096;
double QAM16_SINR_ratio = 31.6228;
double QAM64_SINR_ratio = 316.2278;

double m_ratio = BSK_SINR_ratio;

int main(int argc, char** argv) {

	if (argc != 7) {
		printf(
				"USAGE: it generates reception probability for certain transmit power, transmit antenna gain and receive antenna gain\n");
		printf(
				"SYNOPSIS: ptd2ns2 -Pt <transmit-power> -Gt <transmit-antenna-gain> -Gr <receive-antenna-gain>\n");
		return 1;
	}

	int optind = 1;
	// decode arguments
	while ((optind < argc) && (argv[optind][0] == '-')) {
		string sw = argv[optind];
		if (sw == "-Pt") {
			optind++;
			Pt = atof(argv[optind]);
		} else if (sw == "-Gt") {
			optind++;
			Gt = atof(argv[optind]);
		} else if (sw == "-Gr") {
			optind++;
			Gr = atof(argv[optind]);
		} else {
			cout << "Unknown switch: " << argv[optind] << endl;
		}
		optind++;
	}

	typedef std::map<double, unsigned int> rxPowerMapType;
	typedef std::map<double, double> p_reception_type;

	double step = 100.0;
	double total = 10000.0;
	double max = 1000.0;

	p_reception_type p_reception;

	for (double distance = 0.0; distance <= max; distance += step) {

		for (double i = 0; i < total; i++) {

			double Pr_ = GetPr(true, distance, Gt, Gr, DBmtoW(Pt));

			if (Pr_ >= m_ratio * noise_floor) {
				p_reception[distance]++;
			}

			double round = dround(WtoDBm(Pr_), 1.0);

			//printf("Pt(DBm): %.6f Pr(W): %.6f Round(Pr(DBm)):%f\n", Pt, DBmtoW(Pt), round);
		}
	}

	for (p_reception_type::const_iterator i = p_reception.begin(); i
			!= p_reception.end(); ++i) {
		std::cout << (double) i->first << " " << (double) i->second / total
				<< " " << 0.05 << std::endl;
	}

	return 0;
}
