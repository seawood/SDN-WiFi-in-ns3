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

#ifndef WIFI_ELEMENTS_H
#define WIFI_ELEMENTS_H

#include <ns3/wifi-module.h>
#include "ofswitch13-interface.h"

namespace ns3{

class WifiAp : public SimpleRefCount<WifiAp>
{
public:
	WifiAp (const Address& address);
	void SetChannelInfo (const uint8_t& channel, const uint16_t& frequency,
						 const uint16_t& channelWidth,
						 const WifiPhyStandard& standard = WIFI_PHY_STANDARD_UNSPECIFIED);
	void SetMac48Address (const uint8_t buffer[6]);
	Mac48Address GetMac48Address (void);
	Address GetAddress (void);
	
private:
	Address m_address;  //Ipv4Address
	Mac48Address m_mac48address;
	
	// channel information
	uint8_t m_channelNumber;
	uint16_t m_frequency;
	uint16_t m_channelWidth;
	WifiPhyStandard m_standard;
		  
}; //class WifiAp

/**
* \ingroup ofswitch13
* Inner class to save information of Wifi network, including frequency resource pool
*/
class WifiNetworkStatus : public SimpleRefCount<WifiNetworkStatus>
{
public:
	WifiNetworkStatus ();
	void UpdateFrequencyUsed (Address address, uint16_t frequency, uint16_t width);
	void AddApMac48address (const Mac48Address& mac48address);
	void UpdateChannelQuality(const Address& apAddr, struct chaqua_report* report);
	void UpdateApsInterference (const Address& dstAp, const Address& srcAp, struct chaqua_report* report);
	void GetOneSTA (Address* ap, Mac48Address* sta); //temporary
	void PrintChannelQuality (void); //temporary, when receive reply or trigger
	void UpdateAssocStas (const Address& apAddr, const Mac48Address& staAddr);
	void UpdateDisassocStas (const Address& apAddr, const Mac48Address& staAddr);
	void PrintAssocStatus(void);
	void GetDisassocApSTA(Address& ap, Mac48Address& sta);

        struct ChannelReport {
        	uint64_t packets;     //number of received packets
        	double rxPower_avg;   //average ?double
        	double rxPower_std;   //standard deviation
        };
	typedef std::map<Mac48Address, std::map<Address, struct ChannelReport>> STAsCQMap;
	typedef std::map<Address, std::map<Address, struct ChannelReport>> APsIfMap;
	
	STAsCQMap* GetSTAsCQMap(void);
	Address GetAssocAp (const Mac48Address& sta);
private:
	void InitializeFrequencyUnused ();
	
	typedef std::pair<uint16_t, uint16_t> FrequencyWidthPair;
	std::map<FrequencyWidthPair, std::set<Address>> m_frequencyUsed;
	std::set<FrequencyWidthPair> m_frequencyUnused;
	
	std::set<Mac48Address> m_apsMac48address;
	
	STAsCQMap m_STAsChannelQuality;
	APsIfMap m_APsInterference;
	
	std::map<Address, std::set<Mac48Address>> m_associationMap;
}; // class WifiNetworkStatus

} // namespace ns3

#endif //WIFI_ELEMENTS_H
