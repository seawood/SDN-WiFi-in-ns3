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
 * a SDN-based Wi-Fi network with 4 APs
 * channel configuration: 0s-->(1,1,1)
 *                        10s-->(1,2,3)
 *                        20s-->(1,2,5)
 *                        30s-->(1,6,9)
 *                        40s-->(1,6,11)
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

//NodeId : (samplesNum, rxPowerAvg)
std::unordered_map<uint32_t, std::pair<uint32_t,double>> record;
void MonitorSpectrumRx(bool signalType, 
					   uint32_t senderNodeId,
					   double rxPower,
					   Time duration)
{
	if (record.find(senderNodeId) != record.end())
	{
		record[senderNodeId].first++;
		record[senderNodeId].second += ((rxPower - record[senderNodeId].second)/record[senderNodeId].first);
	}
	else
	{
		record[senderNodeId] = std::make_pair(1, rxPower);
	}
	std::cout << "nodeId:" << senderNodeId << "; rxPower:" << rxPower << "; rxPowerAvg:" << record[senderNodeId].second <<std::endl;
	
}

int
main (int argc, char *argv[])
{
	double simTime = 50;        //Seconds
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
		//LogComponentEnable ("OFSwitch13WifiController", LOG_LEVEL_ALL);
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
	aps.Create (4);
	
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
	Ssid ssid1 = Ssid ("wifi1");
	spectrumPhy.Set ("Frequency", UintegerValue (2412));
	wifiMac.SetType ("ns3::ApWifiMac");
	NetDeviceContainer apWifiDevs = wifi.Install (spectrumPhy, wifiMac, aps);
	
	//mobility configuration
	MobilityHelper mobility;
	
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	positionAlloc->Add (Vector (0.0, distance, 0.0));
	positionAlloc->Add (Vector (distance, distance, 0.0));
	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
	positionAlloc->Add (Vector (distance, 0.0, 0.0));
	mobility.SetPositionAllocator (positionAlloc);
	
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	
	mobility.Install (aps);

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
	
	Config::ConnectWithoutContext ("/NodeList/"+std::to_string(aps.Get(0)->GetId())+
								   "/DeviceList/"+std::to_string(apWifiDevs.Get(0)->GetIfIndex())+
								   "/$ns3::WifiNetDevice/Phy/$ns3::SpectrumWifiPhy/SignalArrival", 
								   MakeCallback (&MonitorSpectrumRx));
	
	// Enable datapath stats and pcap traces at APs and controller(s)
	if (trace)
    {
		of13Helper->EnableOpenFlowPcap ("openflow");
		of13Helper->EnableDatapathStats ("ap-openflow-stats");
		spectrumPhy.EnablePcap ("apWifi", apWifiDevs);
    }
	
	uint16_t interval = 1;
	for (int i = 10; i < 50; i+=10)
	{
		Simulator::Schedule (Seconds(10*i), &OFSwitch13WifiController::ConfigChannelStrategyInterval, 
							 wifiControl, interval);
		interval++;
		std::cout << "ChannelQuality Report from:" << i+1 <<"s to:" << i+9 << "s" <<std::endl;
		for (int j = i+1; j < j+10; ++j)
		{
			Simulator::Schedule (Seconds (j), &OFSwitch13WifiController::ChannelQualityReportStrategy,
								 wifiControl);
		}
		
	}
	
	Simulator::Stop (Seconds (simTime + 1));
	
	// Run the simulation
	Simulator::Run ();
	
	//print simulation result
	std::cout <<"***" <<std::endl;
	std::cout << std::setprecision (4) << std::fixed;
	
	std::cout << std::setw (12) << "nodeId" <<
			  std::setw (12) << "samples" <<
			  std::setw (12) << "rxPower" <<
			  std::endl;
	for (const auto& item : record)
	{
		std::cout << std::setw (12) << item.first <<
				  std::setw (12) << item.second.first <<
				  std::setw (12) <<item.second.second <<
				  std::endl;
	}
	
	Simulator::Destroy ();
	return 0;
}
