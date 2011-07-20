/***************************************************************************
 *   Copyright (C) 2011 by Sergio Vieira                                   *
 *   sergiosvieira@larces.uece.br                                          *
 *   Computer Networks and Security Laboratory (LARCES)                    *
 *   State University of Ceara (UECE/Brazil)                               *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "ns3/core-module.h"
#include "ns3/common-module.h"
#include "ns3/wifi-module.h" 
#include "ns3/mobility-module.h"
#include "ns3/random-variable.h"
#include <iostream>
#include <math.h>

using namespace ns3;

/* Main Program */
double m_distance0 = 1.0;
double m_distance1 = 80.0;
double m_distance2 = 200.0;
double m_exponent0 = 1.9;
double m_exponent1 = 3.8;
double m_exponent2 = 3.8;
double m_referenceLoss = 46.6777;
double m_minDistance = 0.5;
double m_lambda = 300000000.0 / 5.150e9;
double step = 1.0;
double m_systemLoss = 1.0;
double m_m0 = 1.5;
double m_m1 = 0.75;
double m_m2 = 0.75;
const double PI = 3.14159265358979323846;

double getRxPowerFriis(double dist, double txPowerDbm) {
	double distance = dist;
  
	if (distance <= m_minDistance) {
      return txPowerDbm;
    }
	double numerator = m_lambda * m_lambda;
	double denominator = 16 * PI * PI * distance * distance * m_systemLoss;
	double pr = 10 * log10 (numerator / denominator);
	NS_LOG_DEBUG ("distance="<<distance<<"m, attenuation coefficient="<<pr<<"dB");
	return txPowerDbm + pr;
}

double getRxPowerLog(double dist, double txPowerDbm) {
	m_distance1 = 200.0;
	m_distance2 = 500.0;
	double distance = dist;
	NS_ASSERT(distance >= 0);

	// See doxygen comments for the formula and explanation

	double pathLossDb;

	if (distance < m_distance0) {
		pathLossDb = 0;
	} else if (distance < m_distance1) {
		pathLossDb = m_referenceLoss + 10 * m_exponent0 * log10(distance
				/ m_distance0);
	} else if (distance < m_distance2) {
		pathLossDb = m_referenceLoss + 10 * m_exponent0 * log10(m_distance1
				/ m_distance0) + 10 * m_exponent1 * log10(distance
				/ m_distance1);
	} else {
		pathLossDb = m_referenceLoss + 10 * m_exponent0 * log10(m_distance1
				/ m_distance0) + 10 * m_exponent1 * log10(m_distance2
				/ m_distance1) + 10 * m_exponent2 * log10(distance
				/ m_distance2);
	}

	NS_LOG_DEBUG ("ThreeLogDistance distance=" << distance << "m, " <<
			"attenuation=" << pathLossDb << "dB");

	return txPowerDbm - pathLossDb;

}

double getRxPower(double dist, double txPower) {
	ErlangVariable m_erlangRandomVariable;
	GammaVariable m_gammaRandomVariable;

	double distance = dist;

	NS_ASSERT(distance >= 0);

	double m;

	if (distance < m_distance1) {
		m = m_m0;
	} else if (distance < m_distance2) {
		m = m_m1;
	} else {
		m = m_m2;
	}

	// the current power unit is dBm, but Watt is put into the Nakagami /
	// Rayleigh distribution.
	double powerW = pow(10, (txPower - 30) / 10);

	double resultPowerW;

	// switch between Erlang- and Gamma distributions: this is only for
	// speed. (Gamma is equal to Erlang for any positive integer m.)
	unsigned int int_m = static_cast<unsigned int> (floor(m));

	if (int_m == m) {
		resultPowerW = m_erlangRandomVariable.GetValue(int_m, powerW / m);
	} else {
		resultPowerW = m_gammaRandomVariable.GetValue(m, powerW / m);
	}

	double resultPowerDbm = 10 * log10(resultPowerW) + 30;

	return resultPowerDbm;
}

int main(int argc, char **argv) {
	UniformVariable var(0, 1000);

	double txPowerDbm = 21.5;

	double d = 10.0;
	uint32_t seed = 1978;

	CommandLine cmd;
	cmd.AddValue("txp", "txPowerDbm", txPowerDbm);
	cmd.AddValue("d", "Distance", d);
	cmd.AddValue("d0", "d0", m_distance1);
	cmd.AddValue("d1", "d1", m_distance2);
	cmd.AddValue("m0", "m0", m_m0);
	cmd.AddValue("m1", "m1", m_m1);
	cmd.AddValue("m2", "m2", m_m2);
	cmd.AddValue("seed", "seed", m_m2);
	cmd.AddValue("step", "distance increment", step);
	cmd.Parse(argc, argv);

	SeedManager::SetSeed(seed);

	double ntxp = 0.0;

	for (double i = d; i <= 1000.0; i += step) {
		//ntxp = getRxPowerLog(i, txPowerDbm);
		ntxp = getRxPowerFriis(i, txPowerDbm) + 1.0;
		std::cout << i << " " << ntxp << " " << getRxPower(i, ntxp) << std::endl;
	}

	return 0;
}
