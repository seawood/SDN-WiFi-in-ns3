/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 University of Campinas (Unicamp)
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

#include "ns3/log.h"
#include "wifi-elements.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WifiElements");


// definition of class wifiAp

WifiAp::WifiAp (const Address& address)
{
	NS_LOG_FUNCTION (this);
	m_address = address;
}

void
WifiAp::SetChannelInfo (const WifiPhyStandard& standard, const uint8_t& channel,
					 const uint16_t& frequency, const uuint16_t& channelWidth)
{
	NS_LOG_FUNCTION (this << standard << channel << frequency << channelWidth);
	m_standard = standard;
	m_channelNumber = channel;
	m_frequency = frequency;
	m_channelWidth = channelWidth;
	
}



// definition of class wifiNetWorkStatus

WifiNetworkStatus::WifiNetworkStatus()
{
	NS_LOG_FUNCTION (this);
	InitializeFrequencyUnused();
}

void
WifiNetworkStatus::InitializeFrequencyUnused ()
{
	NS_LOG_FUNCTION (this);
	for (auto it : WifiPhy::m_channelToFrequencyWidth)
	{
		if (it->first.second == WIFI_PHY_STANDARD_UNSPECIFIED)
		{
			m_frequencyUnused.insert (it->second);
		}
	}
}
void
WifiNetworkStatus::UpdateFrequencyUsed (Address address, 
										uint16_t frequency, uint16_t width)
{
	NS_LOG_FUNCTION (this << address << frequency << width);
	FrequencyWidthPair pair(frequency, width);
	if (auto it = m_frequencyUsed.find (pair))
	{
		it->second.insert(address);
	}
	else
	{
		m_frequencyUsed.insert (FrequencyWidthPair (pair, address));
		m_frequencyUnused.erase (pair);
	}
	
}



} //namespace ns3
