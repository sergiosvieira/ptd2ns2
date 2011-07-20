#include <iostream>
#include "ranvar.h"
#include <cmath>

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
double lambda = 300000000.0 / 5.150e9;
double L = 1.0; // system Loss
double Pt = 24.5; // em Dbm
double Gt = 1.0;
double Gr = 1.0;

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

/*
 double lambda_; // wavelength (m)
 double CSThresh_; // carrier sense threshold (W) fixed by chipset
 double CPThresh_; // capture threshold
 double RXThresh_; // capture threshold
 double Pt_; // transmitted signal power (W)
 double freq_; // frequency
 double L_; // system loss factor
 double HeaderDuration_; // preamble+SIGNAL
 */
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

int main(int argc, char** argv) {
	double step = 50.0;
	for (double i = 100.0; i <= 1000.0; i += step) {
		//cout << i << endl;
		std::cout << i << " " << Pt << " " << GetPr(true, i, Gt, Gr, Pt) << std::endl;
	}
	return 0;
}
