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
 * AP0 and AP1 connected to a single OpenFlow switch. STA0 is served by AP0. STA1 is served by AP1.
 * The switch and two APs are managed by the default learning controller application.
 * STA0 and STA1 communicates with each other via the wired link.
 * 
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
	uint16_t simTime = 10;
	bool verbose = true;
	bool trace = true;
	std::string errorModelType = "ns3::NistErrorRateModel";
	double distance = 50;

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
		OFSwitch13Helper::EnableDatapathLogs ();
		LogComponentEnable ("OFSwitch13Interface", LOG_LEVEL_ALL);
		LogComponentEnable ("OFSwitch13Device", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13Port", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13Queue", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13SocketHandler", LOG_LEVEL_ALL);
		LogComponentEnable ("OFSwitch13Controller", LOG_LEVEL_ALL);
		//LogComponentEnable ("OFSwitch13LearningController", LOG_LEVEL_ALL);
		LogComponentEnable ("OFSwitch13Helper", LOG_LEVEL_ALL);
		LogComponentEnable ("OFSwitch13InternalHelper", LOG_LEVEL_ALL);
		LogComponentEnable ("WifiNetDevice", LOG_LEVEL_ALL);
		//LogComponentEnable ("CsmaNetDevice", LOG_LEVEL_ALL);
		//LogComponentEnable ("Simulator", LOG_LEVEL_ALL);
		LogComponentEnable ("OFSwitch13WifiController", LOG_LEVEL_ALL);
		LogComponentEnable ("WifiElements", LOG_LEVEL_ALL);
		LogComponentEnable ("WifiPhy", LOG_LEVEL_ALL);
		LogComponentEnable ("SpectrumWifiPhy", LOG_LEVEL_ALL);
    }
	

	// Enable checksum computations (required by OFSwitch13 module)
	GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

	// Create two AP nodes
	NodeContainer aps;
	aps.Create (2);

	// Create the switch node
	Ptr<Node> switchNode = CreateObject<Node> ();

	// Use the CsmaHelper to connect AP nodes to the switch node
	CsmaHelper csmaHelper;
	csmaHelper.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
	csmaHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
	//csmaHelper.SetDeviceAttribute ("EncapsulationMode", EnumValue (CsmaNetDevice::LLC));

	NetDeviceContainer apDevices;
	NetDeviceContainer switchPorts;
	for (size_t i = 0; i < aps.GetN (); i++)
    {
		NodeContainer pair (aps.Get (i), switchNode);
		NetDeviceContainer link = csmaHelper.Install (pair);
		apDevices.Add (link.Get (0));
		switchPorts.Add (link.Get (1));
    }

	// Install WifiNetDevice on AP0,AP1,STA0 and STA1, configure the channel
	NodeContainer stas;
	stas.Create (2);
	NetDeviceContainer staDevs;
	NetDeviceContainer apWifiDevs;
	
	Config::SetDefault ("ns3::WifiPhy::CcaMode1Threshold", DoubleValue (-62.0));
	//Config::SetDefault ("ns3::WifiPhy::Frequency", UintegerValue (2417));
	//Config::SetDefault ("ns3::WifiPhy::ChannelWidth", UintegerValue (20));
	//Config::SetDefault ("ns3::WifiPhy::ChannelNumber", UintegerValue (2));
	
	// spectrum channel configuration
	Ptr<MultiModelSpectrumChannel> spectrumChannel
		= CreateObject<MultiModelSpectrumChannel> ();
	Ptr<FriisPropagationLossModel> lossModel
		= CreateObject<FriisPropagationLossModel> ();
	//lossModel->SetFrequency (5.180e9);
	spectrumChannel->AddPropagationLossModel (lossModel);
	Ptr<ConstantSpeedPropagationDelayModel> delayModel
		= CreateObject<ConstantSpeedPropagationDelayModel> ();
	spectrumChannel->SetPropagationDelayModel (delayModel);
	
	// spectrum phy configuration
	SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
	spectrumPhy.SetChannel (spectrumChannel);
	spectrumPhy.SetErrorRateModel (errorModelType);
	//spectrumPhy.Set ("Frequency", UintegerValue (5180));
	spectrumPhy.Set ("TxPowerStart", DoubleValue (10)); // dBm  (1.26 mW)
	spectrumPhy.Set ("TxPowerEnd", DoubleValue (10));
	spectrumPhy.Set ("ShortGuardEnabled", BooleanValue (false));
	//spectrumPhy.Set ("ChannelWidth", UintegerValue (20));

	WifiHelper wifi;
	//wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
	//StringValue DataRate = StringValue ("HtMcs0");
	//wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate,
	//"ControlMode", DataRate);
	//mac configuration
	WifiMacHelper wifiMac;
	Ssid ssid1 = Ssid ("wifi1");
	
	//ap0 and sta0 use 2412MHz(channel 1), ap1 and sta1 use 2417MHz(channel 2)
	wifiMac.SetType ("ns3::StaWifiMac",
					 "ActiveProbing", BooleanValue (true),
					 "Ssid", SsidValue (ssid1));
	spectrumPhy.Set ("Frequency", UintegerValue (2412));
	staDevs.Add (wifi.Install (spectrumPhy, wifiMac, stas.Get(0)));
	
	wifiMac.SetType ("ns3::ApWifiMac",
					 "Ssid", SsidValue (ssid1));
	apWifiDevs.Add (wifi.Install (spectrumPhy, wifiMac, aps.Get(0)));
	
	Ssid ssid2 = Ssid ("wifi2");
	spectrumPhy.Set ("Frequency", UintegerValue (2417));
	wifiMac.SetType ("ns3::StaWifiMac",
					 "ActiveProbing", BooleanValue (true),
					 "Ssid", SsidValue (ssid2));
	staDevs.Add (wifi.Install (spectrumPhy, wifiMac, stas.Get(1)));
	
	wifiMac.SetType ("ns3::ApWifiMac",
					 "Ssid", SsidValue (ssid2));
	apWifiDevs.Add (wifi.Install (spectrumPhy, wifiMac, aps.Get(1)));
	
	//mobility configuration
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
	positionAlloc->Add (Vector (distance, 0.0, 0.0));
	positionAlloc->Add (Vector (0.0, distance, 0.0));
	positionAlloc->Add (Vector (distance, distance, 0.0));
	mobility.Install (aps);
	mobility.Install (stas);

	// Create the controller node
	Ptr<Node> controllerNode = CreateObject<Node> ();

	// Configure the OpenFlow network domain
	Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper> ();
	Ptr<OFSwitch13WifiController> wifiControl = CreateObject<OFSwitch13WifiController> ();
	of13Helper->InstallController (controllerNode, wifiControl);
	of13Helper->InstallSwitch (switchNode, switchPorts);
	for (size_t i = 0; i < aps.GetN(); i++)
	{
		NetDeviceContainer tmp;
		tmp.Add (apDevices.Get (i));
		tmp.Add (apWifiDevs.Get(i));
		of13Helper->InstallSwitch (aps.Get (i), tmp);
	}
	of13Helper->CreateOpenFlowChannels ();
	
	// Install the TCP/IP stack into STA nodes
	InternetStackHelper internet;
	internet.Install (stas);

	// Set IPv4 addresses for STAs
	Ipv4AddressHelper ipv4helpr;
	ipv4helpr.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer staIpIfaces;
	staIpIfaces = ipv4helpr.Assign (staDevs);

	// Configure ping application between STAs, STA0 ping STA1
	V4PingHelper pingHelper = V4PingHelper (staIpIfaces.GetAddress (1));
	pingHelper.SetAttribute ("Verbose", BooleanValue (true));
	ApplicationContainer pingApps = pingHelper.Install (stas.Get (0));
	pingApps.Start (Seconds (1));
	
	Config::ConnectWithoutContext ("/NodeList/"+std::to_string(aps.Get(0)->GetId())+
								   "/DeviceList/"+std::to_string(apWifiDevs.Get(0)->GetIfIndex())+
								   "/$ns3::WifiNetDevice/Phy/$ns3::SpectrumWifiPhy/SignalArrival", 
								   MakeCallback (&MonitorSpectrumRx));
	
	// Enable datapath stats and pcap traces at STAs, APs, switch(es), and controller(s)
	if (trace)
    {
		of13Helper->EnableOpenFlowPcap ("openflow");
		of13Helper->EnableDatapathStats ("switch-stats");
		csmaHelper.EnablePcap ("switch", switchPorts, true);
		csmaHelper.EnablePcap ("apCsma", apDevices);
		spectrumPhy.EnablePcap ("apWifi", apWifiDevs);
		spectrumPhy.EnablePcap ("sta", staDevs);
    }
	
	Simulator::Schedule (Seconds (3), &OFSwitch13WifiController::ChannelQualityReportStrategy,
						 wifiControl);
	Simulator::Schedule (Seconds (6), &OFSwitch13WifiController::ChannelQualityTriggerStrategy,
	wifiControl);

	Simulator::Stop (Seconds (simTime));
	
	// Run the simulation
	Simulator::Run ();
	
	//print simulation result
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


