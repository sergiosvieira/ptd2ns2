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

/*
 * Definitive Nakagami Wifi
 * User Arguments:
    --st: Simulation Time
    --ss: Simulation Seed
    --dn: Distance between nodes
*/
 
#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/common-module.h"
#include "ns3/node-module.h"
#include "ns3/helper-module.h"
#include "ns3/mobility-module.h"
#include "ns3/contrib-module.h"
#include "ns3/wifi-module.h" 
#include "ns3/v4ping-helper.h"

NS_LOG_COMPONENT_DEFINE("DefinitiveNakagamiWifi");

using namespace ns3;

/* Class */
class Experiment {
public:
	Experiment();
	void configure(int argc, char **argv);
	void run();
private:
	NodeContainer nodes_;
	NetDeviceContainer devices_;
	Ipv4InterfaceContainer interfaces_;
	double simulation_time_;
	double simulation_seed_;
	double distance_;
	void createNodes();
	void createDevices();
	void createMobility();
	void installInternetStack();
	void installApplications();
};

/* Public Methods */
Experiment::Experiment() {
	simulation_time_ = 10.0;
	simulation_seed_ = 1978.0;
	distance_ = 100.0;
}

void Experiment::configure(int argc, char **argv) {
	CommandLine cmd;
	cmd.AddValue("st", "Simulation Time", simulation_time_);
	cmd.AddValue("ss", "Simulation Seed", simulation_seed_);
	cmd.AddValue("dn", "Distance between nodes", distance_);
	cmd.Parse(argc, argv);
	
	SeedManager::SetSeed(simulation_seed_);
}

void Experiment::run() {
	createNodes();
	createMobility();
	createDevices();
	installInternetStack();
	installApplications();
	Simulator::Stop(Seconds(simulation_time_));
	Simulator::Run();
	Simulator::Destroy();	
}

/* Private Methods */
void Experiment::createNodes() {
	NS_LOG_INFO("creating nodes...");
	nodes_.Create(2);
}

void Experiment::createDevices() {
	NS_LOG_INFO("creating devices...");
	/* Creating Physical Layer */
	// Channel
	YansWifiChannelHelper chn;// = YansWifiChannelHelper::Default();
	chn.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	chn.AddPropagationLoss ("ns3::FriisPropagationLossModel");
	chn.AddPropagationLoss("ns3::NakagamiPropagationLossModel");	
	// Physical
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
	phy.Set("TxPowerStart", DoubleValue(16.0));
	phy.Set("TxPowerEnd", DoubleValue(16.0));
	phy.Set("TxPowerLevels", UintegerValue(1));
	double threshold = -89.6779;
	// 100m threshold = -69.6779;
	// 200m threshold = -75.6985
	// 300m threshold = -79.2203;
	// 500m threshold = -83.6573;
	// 800m threshold = -87.7397
	// 1000m threshold = -89.6779
	phy.Set("EnergyDetectionThreshold", DoubleValue(threshold));
	phy.Set("CcaMode1Threshold", DoubleValue(threshold + 3.0));
	phy.Set("RxNoiseFigure", DoubleValue(4.0));
	phy.SetChannel(chn.Create());
	/* Creating MAC Layer */
	NqosWifiMacHelper mac = NqosWifiMacHelper::Default();
	mac.SetType("ns3::AdhocWifiMac");
	/* Wifi */
	WifiHelper wifi = WifiHelper::Default();
	wifi.SetStandard(WIFI_PHY_STANDARD_80211p_CCH);
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager");
	devices_ = wifi.Install(phy, mac, nodes_);
}

void Experiment::createMobility() {
	NS_LOG_INFO("creating mobility...");
	Ptr<ListPositionAllocator> lpa = CreateObject<ListPositionAllocator>();
	MobilityHelper mh;
	for (uint32_t i = 0; i < nodes_.GetN(); i++) {
		lpa->Add(Vector(i * distance_, 0.0, 0.0));
		NS_LOG_INFO("\t(" << i * distance_ << ", 0.0, 0.0)\n");
	}
	mh.SetPositionAllocator(lpa);
	mh.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mh.Install(nodes_);
}

void Experiment::installInternetStack() {
	AodvHelper aodv;
	InternetStackHelper stack;
	stack.SetRoutingHelper(aodv);
	stack.Install(nodes_);
	Ipv4AddressHelper address;
	address.SetBase("10.0.0.0", "255.0.0.0");
	interfaces_ = address.Assign(devices_);
}

void Experiment::installApplications() {
	V4PingHelper ping(interfaces_.GetAddress(1));
	ping.SetAttribute("Verbose", BooleanValue(true));
	ApplicationContainer p = ping.Install(nodes_.Get(0));
	p.Start(Seconds(0));
	p.Stop(Seconds(simulation_time_) - Seconds(0.001));
}

/* Main Program */
int main(int argc, char **argv) {
	Experiment ex;
	ex.configure(argc, argv);
	ex.run();
	return 0;
}

 
