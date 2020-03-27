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

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/ofswitch13-module.h>
#include <ns3/internet-apps-module.h>
#include <ns3/wifi-module.h>
#include "ns3/mobility-helper.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
  uint16_t simTime = 10;
  bool verbose = true;
  bool trace = true;

  // Configure command line parameters
  CommandLine cmd;
  cmd.AddValue ("simTime", "Simulation time (seconds)", simTime);
  cmd.AddValue ("verbose", "Enable verbose output", verbose);
  cmd.AddValue ("trace", "Enable datapath stats and pcap traces", trace);
  cmd.Parse (argc, argv);

  if (verbose)
    {
      OFSwitch13Helper::EnableDatapathLogs ();
      //LogComponentEnable ("OFSwitch13Interface", LOG_LEVEL_ALL);
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
  
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
  //wifi.SetStandard (WIFI_PHY_STANDARD_UNSPECIFIED);
  Config::SetDefault ("ns3::WifiPhy::Frequency", UintegerValue (2417));
  Config::SetDefault ("ns3::WifiPhy::ChannelWidth", UintegerValue (20));
  Config::SetDefault ("ns3::WifiPhy::ChannelNumber", UintegerValue (2));
  
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiPhy.SetChannel (wifiChannel.Create ());
  
  WifiMacHelper wifiMac;
  Ssid ssid1 = Ssid ("wifi1");
  wifiMac.SetType ("ns3::StaWifiMac",
                   "ActiveProbing", BooleanValue (true),
                   "Ssid", SsidValue (ssid1));
  staDevs.Add (wifi.Install (wifiPhy, wifiMac, stas.Get(0)));
  
  Ssid ssid2 = Ssid ("wifi2");
  wifiMac.SetType ("ns3::StaWifiMac",
                   "ActiveProbing", BooleanValue (true),
                   "Ssid", SsidValue (ssid2));
  staDevs.Add (wifi.Install (wifiPhy, wifiMac, stas.Get(1)));
  
  wifiMac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid1));
  apWifiDevs.Add (wifi.Install (wifiPhy, wifiMac, aps.Get(0)));
  
  wifiMac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid2));
  apWifiDevs.Add (wifi.Install (wifiPhy, wifiMac, aps.Get(1)));
  
  MobilityHelper mobility;
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

  // Enable datapath stats and pcap traces at STAs, APs, switch(es), and controller(s)
  if (trace)
    {
      of13Helper->EnableOpenFlowPcap ("openflow");
      of13Helper->EnableDatapathStats ("switch-stats");
      csmaHelper.EnablePcap ("switch", switchPorts, true);
      csmaHelper.EnablePcap ("apCsma", apDevices);
	  wifiPhy.EnablePcap ("apWifi", apWifiDevs);
	  wifiPhy.EnablePcap ("sta", staDevs);
    }
	
	Simulator::Schedule (Seconds (3), &OFSwitch13WifiController::ConfigChannelStrategy,
						wifiControl);

  // Run the simulation
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();
}
