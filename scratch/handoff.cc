/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 Peking University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: XieYingying <xyynku@163.com>
 *
 * a SDN-based Wi-Fi network with 2 APs and 1 STA
 * STA is handoffed from AP1 to AP2
 */
#include <iomanip>
#include <string>
#include <unordered_map>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/ofswitch13-module.h>
#include <ns3/internet-apps-module.h>
#include <ns3/wifi-module.h>
#include <ns3/spectrum-module.h>
#include "ns3/mobility-helper.h"
#include "ns3/udp-client-server-helper.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
	double simTime = 20;        //Seconds
	bool verbose = true;
	bool trace = true;
	std::string errorModelType = "ns3::NistErrorRateModel";
	double distance = 5;        //meters

	// Configure command line parameters
	CommandLine cmd;
	cmd.AddValue ("simTime", "Simulation time (seconds)", simTime);
	cmd.AddValue ("verbose", "Enable verbose output", verbose);
	cmd.AddValue ("trace", "Enable datapath stats and pcap traces", trace);
	cmd.AddValue ("errorModelType", "select ns3::NistErrorRateModel or ns3::YansErrorRateModel", errorModelType);
	cmd.AddValue ("distance", "distance between nodes", distance);
	cmd.Parse (argc, argv);

	if (verbose)
    {
		//OFSwitch13Helper::EnableDatapathLogs ();
		//LogComponentEnable ("OFSwitch13Interface", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13Device", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13Port", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13Queue", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13SocketHandler", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13Controller", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13LearningController", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13Helper", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13InternalHelper", LOG_LEVEL_ALL);
		//LogComponentEnable ("WifiNetDevice", LOG_LEVEL_ALL);
		//LogComponentEnable ("CsmaNetDevice", LOG_LEVEL_ALL);
		//LogComponentEnable ("Simulator", LOG_LEVEL_ALL);
		LogComponentEnable ("OFSwitch13WifiController", LOG_LEVEL_ALL);
		LogComponentEnable ("WifiElements", LOG_LEVEL_WARN);
		//LogComponentEnable ("WifiPhy", LOG_LEVEL_ALL);
		//LogComponentEnable ("SpectrumWifiPhy", LOG_LEVEL_ALL);
		//LogComponentEnable ("UdpServer", LOG_LEVEL_ALL);
		//LogComponentEnable ("UdpClient", LOG_LEVEL_ALL);
	    //LogComponentEnable ("PropagationLossModel", LOG_LEVEL_ALL);
    }
	

	// Enable checksum computations (required by OFSwitch13 module)
	GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

	// Create two AP nodes
	NodeContainer aps;
	aps.Create (2);
	NodeContainer stas;
	stas.Create (1);
	
	Config::SetDefault ("ns3::WifiPhy::CcaMode1Threshold", DoubleValue (-62.0));
	//Config::SetDefault ("ns3::WifiPhy::Frequency", UintegerValue (2417));
	//Config::SetDefault ("ns3::WifiPhy::ChannelWidth", UintegerValue (20));
	//Config::SetDefault ("ns3::WifiPhy::ChannelNumber", UintegerValue (2));
	
	// spectrum channel configuration
	Ptr<MultiModelSpectrumChannel> spectrumChannel
		= CreateObject<MultiModelSpectrumChannel> ();
	Ptr<RandomPropagationLossModel> lossModel
		= CreateObject<RandomPropagationLossModel> ();
	Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();
	x -> SetAttribute ("Min", DoubleValue(0.0));
	x -> SetAttribute ("Max", DoubleValue(20.0));
	lossModel -> SetAttribute ("Variable", PointerValue(x));
	//lossModel->SetFrequency (5.180e9);
	spectrumChannel->AddPropagationLossModel (lossModel);
	Ptr<ConstantSpeedPropagationDelayModel> delayModel
		= CreateObject<ConstantSpeedPropagationDelayModel> ();
	spectrumChannel->SetPropagationDelayModel (delayModel);
	
	// spectrum phy configuration
	SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
	spectrumPhy.SetChannel (spectrumChannel);
	spectrumPhy.SetErrorRateModel (errorModelType);
	spectrumPhy.Set ("TxPowerStart", DoubleValue (100)); // dBm  (1.26 mW)
	spectrumPhy.Set ("TxPowerEnd", DoubleValue (100));
	spectrumPhy.Set ("ShortGuardEnabled", BooleanValue (false));
	spectrumPhy.Set ("ChannelWidth", UintegerValue (20));

	WifiHelper wifi;
	//wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
	//StringValue DataRate = StringValue("HtMcs0");
	//double datarate = 6.5; //?
	//wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate,
	//							  "ControlMode", DataRate);
	
	//mac configuration
	WifiMacHelper wifiMac;
	NetDeviceContainer apWifiDevs;
	NetDeviceContainer staWifiDevs;
	spectrumPhy.Set ("Frequency", UintegerValue (2412));
	
	Ssid ssid = Ssid ("wifi1");
	wifiMac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));
	apWifiDevs.Add(wifi.Install (spectrumPhy, wifiMac, aps.Get(0)));
	apWifiDevs.Add(wifi.Install (spectrumPhy, wifiMac, aps.Get(1)));
	wifiMac.SetType ("ns3::StaWifiMac",
					 "ActiveProbing", BooleanValue (true),
					 "Ssid", SsidValue (ssid));
	staWifiDevs.Add (wifi.Install (spectrumPhy, wifiMac, stas.Get(0)));
	
	//mobility configuration
	MobilityHelper mobility;
	
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
	positionAlloc->Add (Vector (distance*2, 0.0, 0.0));
	positionAlloc->Add (Vector (distance, 0.0, 0.0));
	mobility.SetPositionAllocator (positionAlloc);
	
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	
	mobility.Install (aps);
	mobility.Install (stas);

	// Create the controller node
	Ptr<Node> controllerNode = CreateObject<Node> ();

	// Configure the OpenFlow network domain
	Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper> ();
	Ptr<OFSwitch13WifiController> wifiControl = CreateObject<OFSwitch13WifiController> ();
	of13Helper->InstallController (controllerNode, wifiControl);
	for (size_t i = 0; i < aps.GetN(); i++)
	{
		NetDeviceContainer tmp;
		tmp.Add (apWifiDevs.Get(i));
		of13Helper->InstallSwitch (aps.Get (i), tmp);
	}
	of13Helper->CreateOpenFlowChannels ();
	
	
	// Enable datapath stats and pcap traces at APs and controller(s)
	if (trace)
    {
		of13Helper->EnableOpenFlowPcap ("openflow");
		of13Helper->EnableDatapathStats ("ap-openflow-stats");
		spectrumPhy.EnablePcap ("apWifi", apWifiDevs);
		spectrumPhy.EnablePcap ("staWifi", staWifiDevs);
    }
	
	Simulator::Schedule(Seconds(5), &OFSwitch13WifiController::PrintAssocStatus,
					   wifiControl);
	Simulator::Schedule (Seconds(10), &OFSwitch13WifiController::ConfigAssocStrategy,
						 wifiControl);
	Simulator::Schedule(Seconds(15), &OFSwitch13WifiController::PrintAssocStatus,
						wifiControl);
	Simulator::Stop (Seconds (simTime + 1));
	
	// Run the simulation
	Simulator::Run ();
	
	Simulator::Destroy ();
	return 0;
}

