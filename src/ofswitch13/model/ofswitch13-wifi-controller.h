/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 Peiking University
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
 * Author: xie yingying <xyynku@163.com>
 */

#ifndef OFSWITCH13_WIFI_CONTROLLER_H
#define OFSWITCH13_WIFI_CONTROLLER_H

#include "ofswitch13-learning-controller.h"

namespace ns3 {

class OFSwitch13WifiController : public OFSwitch13LearningController
{
public:
	OFSwitch13WifiController ();
	virtual ~OFSwitch13WifiController ();
	static TypeId GetTypeId (void);

	/** Destructor implementation */
	virtual void DoDispose ();
	
	virtual ofl_err HandleExperimenterMsg (
		struct ofl_exp_wifi_msg_header *msg, Ptr<const RemoteSwitch> swtch,
		uint32_t xid);
	
	virtual ofl_err HandleFeaturesReplyWifi (Ptr<const RemoteSwitch> swtch);
	
	void ConfigChannelStrategy (void);
	struct OneReport {
		Mac48Address address;
		uint64_t packets;     //number of received packets
		double rxPower_avg;   //average ?double
		double rxPower_std;   //standard deviation
	};
	void ChannelQualityReportStrategy (void);
	void ChannelQualityTriggerStrategy (void); 
	void ConfigChannelStrategyInterval (uint16_t interval);
	void ConfigAssocStrategy (void); // two AP, one STA scenario
	void ConfigAssocLBStrategy (void); //for RSSI based load balance 
	void PrintAssocStatus(void);
	void PrintChannelquality(void);
private:
	void ConfigChannel (const Address& address, const uint8_t& channelNumber,
						const uint16_t frequency, const uint16_t& channelWidth);
	
	void RequestChannelQuality (const Address& address, const Mac48Address& mac48address);
	void SetChannelQualityTrigger (const Address& address, const std::vector<struct OneReport>& triggers);
	
	void DisassocSTA (const Address& ap, const Mac48Address& sta);
	
	/** Map to store Wifi AP info by Address */
	typedef std::map <Address, Ptr<WifiAp>> WifiApsMap_t;
	WifiApsMap_t    m_wifiApsMap;
	typedef std::map <Mac48Address, Ptr<WifiAp>> WifiApsMac48Map_t;
	WifiApsMac48Map_t m_wifiApsMac48Map;
	
	Ptr<WifiNetworkStatus> m_wifiNetworkStatus;
	std::map<Address, Address> AssocControlMap;
};

}  //namespace ns3



#endif  // OFSWITCH13_WIFI_CONTROLLER_H
