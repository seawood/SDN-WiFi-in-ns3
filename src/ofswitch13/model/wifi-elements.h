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

namespace ns3{

class WifiAp : public SimpleRefCount<WifiAp>
{
public:
	WifiAp (const Address& address);
	void SetChannelInfo (const uint8_t& channel, const uint16_t& frequency,
						 const uint16_t& channelWidth,
						 const WifiPhyStandard& standard = WIFI_PHY_STANDARD_UNSPECIFIED);
	
	
private:
	Address m_address;
	
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
	
	  
private:
	void InitializeFrequencyUnused ();
	
	typedef std::pair<uint16_t, uint16_t> FrequencyWidthPair;
	
	std::map<FrequencyWidthPair, std::set<Address>> m_frequencyUsed;
	std::set<FrequencyWidthPair> m_frequencyUnused;
	  
	  
}; // class WifiNetworkStatus

} // namespace ns3

#endif //WIFI_ELEMENTS_H
