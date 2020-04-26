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

WifiAp::WifiAp (const Address& address):
	m_address(address)
{
	NS_LOG_FUNCTION (this);
}

void
WifiAp::SetChannelInfo (const uint8_t& channel, const uint16_t& frequency,
						const uint16_t& channelWidth,
						const WifiPhyStandard& standard)
{
	NS_LOG_FUNCTION (this << standard << channel << frequency << channelWidth);
	m_standard = standard;
	m_channelNumber = channel;
	m_frequency = frequency;
	m_channelWidth = channelWidth;
	
}

void
WifiAp::SetMac48Address (const uint8_t buffer[6])
{
	NS_LOG_FUNCTION (this);
	m_mac48address.CopyFrom (buffer);
}

Mac48Address
WifiAp::GetMac48Address (void)
{
	NS_LOG_FUNCTION (this);
	return m_mac48address;
}
Address
WifiAp::GetAddress (void)
{
	NS_LOG_FUNCTION (this);
	return m_address;
}

// definition of class wifiNetWorkStatus

WifiNetworkStatus::WifiNetworkStatus()
{
	NS_LOG_FUNCTION (this);
	InitializeFrequencyUnused();
}

void
WifiNetworkStatus::AddApMac48address (const Mac48Address& mac48address)
{
	NS_LOG_FUNCTION (this);
	m_apsMac48address.insert (mac48address);
}

void
WifiNetworkStatus::UpdateChannelQuality(const Address& apAddr, 
										struct chaqua_report* report)
{
	Mac48Address addr48;
	addr48.CopyFrom (report->mac48address);
	NS_LOG_FUNCTION (this << addr48<< ";" <<report->packets << ";"
			<< report->rxPower_avg << ";" << report->rxPower_std);
	auto item = m_STAsChannelQuality.find(addr48);
	if (item == m_STAsChannelQuality.end()) //new STA
	{
		std::map<Address, struct ChannelReport> newMap;
		m_STAsChannelQuality[addr48] = newMap;
		NS_LOG_DEBUG ("insert new STA into CQM: " << addr48);
	}
	if (m_STAsChannelQuality[addr48].find (apAddr) != m_STAsChannelQuality[addr48].end())
	{
		m_STAsChannelQuality[addr48][apAddr].packets = report->packets;
		m_STAsChannelQuality[addr48][apAddr].rxPower_avg = report->rxPower_avg;
		m_STAsChannelQuality[addr48][apAddr].rxPower_std = report->rxPower_std;
		NS_LOG_DEBUG("update CQM:");
	}
	else
	{
		struct ChannelReport newChannelReport;
		newChannelReport.packets = report->packets;
		newChannelReport.rxPower_avg = report->rxPower_avg;
		newChannelReport.rxPower_std = report->rxPower_std;
		m_STAsChannelQuality[addr48].insert(std::make_pair(apAddr, newChannelReport));
		NS_LOG_DEBUG ("update CQM with new AP:" << m_STAsChannelQuality[addr48][apAddr].packets);
	}
	//PrintChannelQuality();
	
}

void 
WifiNetworkStatus::UpdateApsInterference (const Address& dstAp, 
							const Address& srcAp, 
							struct chaqua_report* report)
{
	NS_LOG_FUNCTION (this);
	auto item = m_APsInterference.find(srcAp);
	if (item == m_APsInterference.end())
	{
		std::map<Address, struct ChannelReport> newMap;
		m_APsInterference[srcAp] = newMap;
	}
	if (m_APsInterference[srcAp].find(dstAp) != m_APsInterference[srcAp].end())
	{
		m_APsInterference[srcAp][dstAp].packets = report->packets;
		m_APsInterference[srcAp][dstAp].rxPower_avg = report->rxPower_avg;
		m_APsInterference[srcAp][dstAp].rxPower_std = report->rxPower_std;
	}
	else
	{
		struct ChannelReport newChannelReport;
		newChannelReport.packets = report->packets;
		newChannelReport.rxPower_avg = report->rxPower_avg;
		newChannelReport.rxPower_std = report->rxPower_std;
		m_APsInterference[srcAp][dstAp] = newChannelReport;
	}
	//PrintChannelQuality();
}

void
WifiNetworkStatus::InitializeFrequencyUnused ()
{
	NS_LOG_FUNCTION (this);
	for (auto it : WifiPhy::GetChannelToFrequencyWidthMap())
	{
		if (it.first.second == WIFI_PHY_STANDARD_UNSPECIFIED)
		{
			m_frequencyUnused.insert (it.second);
		}
	}
	NS_LOG_INFO("m_frequencyUnused.size():" << m_frequencyUnused.size());
}
void
WifiNetworkStatus::UpdateFrequencyUsed (Address address, 
										uint16_t frequency, uint16_t width)
{
	NS_LOG_FUNCTION (this << address << frequency << width);
	FrequencyWidthPair pair(frequency, width);
	if (m_frequencyUsed.find (pair) != m_frequencyUsed.end())
	{
		NS_LOG_INFO("find in m_frequencyUsed");
		m_frequencyUsed[pair].insert(address);
	}
	else
	{
		std::set<Address> addr;
		addr.insert(address);
		m_frequencyUsed.insert(std::make_pair(pair,addr));
		//m_frequencyUsed[pair] = addr;
		m_frequencyUnused.erase (pair);
	}
	
}

void
WifiNetworkStatus::GetOneSTA (Address* ap, Mac48Address* sta)
{
	NS_LOG_FUNCTION(this);
	if (m_STAsChannelQuality.empty())
	{
		NS_LOG_ERROR("no sta");
		return;
	}
	auto item = m_STAsChannelQuality.begin();
	*sta = item->first;
	*ap = item->second.begin()->first;
}

void 
WifiNetworkStatus::PrintChannelQuality (void)
{
	NS_LOG_FUNCTION(this);
	NS_LOG_INFO ("m_STAsChannelQuality final report:");
	for (auto itr = m_STAsChannelQuality.begin(); itr != m_STAsChannelQuality.end(); ++itr) {
		NS_LOG_INFO ("STA : " << itr->first << "****" << itr->second.size());
		for (auto item = itr->second.begin(); item != itr->second.end(); ++item)
		{
			Ipv4Address apIpv4 = InetSocketAddress::ConvertFrom(item->first).GetIpv4();
			NS_LOG_INFO("in AP:" << apIpv4 << "channel info: " <<
						item->second.packets << ";" << 
						item->second.rxPower_avg << ";" <<
						item->second.rxPower_std);
		}
	}
	NS_LOG_WARN ("m_APsInterference final report:" << Simulator::Now());
	for (auto itr = m_APsInterference.begin(); itr != m_APsInterference.end(); ++itr)
	{
		Ipv4Address ap1Ipv4 = InetSocketAddress::ConvertFrom(itr->first).GetIpv4();
		//NS_LOG_WARN ("AP : " << ap1Ipv4 << "****" << itr->second.size());
		for (auto item = itr->second.begin(); item != itr->second.end(); ++item)
		{
			Ipv4Address ap2Ipv4 = InetSocketAddress::ConvertFrom(item->first).GetIpv4();
			NS_LOG_WARN(ap1Ipv4 << " -> " << ap2Ipv4 << "; time" << Simulator::Now() <<
						"; channel info: " <<
						item->second.packets << ":" << 
						item->second.rxPower_avg << ":" <<
						item->second.rxPower_std);
			
		}
	}
}

void
WifiNetworkStatus::UpdateAssocStas (const Address& apAddr, const Mac48Address& staAddr)
{
	NS_LOG_FUNCTION (this);
	Ipv4Address ap = InetSocketAddress::ConvertFrom(apAddr).GetIpv4();
	NS_LOG_INFO ("ap:" << ap << ", assoc sta:" << staAddr);
	if (m_associationMap.find(apAddr) != m_associationMap.end())
	{
		m_associationMap[apAddr].insert(staAddr);
	}
	else
	{
		std::set<Mac48Address> stas;
		stas.insert(staAddr);
		m_associationMap[apAddr] = stas;
	}
}

void
WifiNetworkStatus::UpdateDisassocStas (const Address& apAddr, const Mac48Address& staAddr)
{
	NS_LOG_FUNCTION (this);
	Ipv4Address ap = InetSocketAddress::ConvertFrom(apAddr).GetIpv4();
	NS_LOG_INFO ("ap:" << ap << ", disassoc sta:" << staAddr);
	NS_ASSERT (m_associationMap.find(apAddr) != m_associationMap.end());
	NS_ASSERT (m_associationMap[apAddr].find(staAddr) != m_associationMap[apAddr].end());
	m_associationMap[apAddr].erase(staAddr);
	if (m_associationMap[apAddr].size() == 0)
	{
		m_associationMap.erase(apAddr);
	}
}

void 
WifiNetworkStatus::PrintAssocStatus(void)
{
	NS_LOG_FUNCTION(this);
	for (auto itr = m_associationMap.begin(); itr != m_associationMap.end(); ++itr)
	{
		Ipv4Address ap = InetSocketAddress::ConvertFrom(itr->first).GetIpv4();
		NS_LOG_INFO("AP:" << ap);
		for (auto i = itr->second.begin(); i != itr->second.end(); i++)
		{
			NS_LOG_INFO("STA" << *i);
		}
	}
}

void
WifiNetworkStatus::GetDisassocApSTA(Address& ap, Mac48Address& sta)
{
	NS_LOG_FUNCTION(this);
	ap = m_associationMap.begin()->first;
	sta = *(m_associationMap.begin()->second.begin());
}

STAsCQMap*
WifiNetworkStatus::GetSTAsCQMap(void)
{
	NS_LOG_FUNCTION (this);
	return &m_STAsChannelQuality;
}

Address 
WifiNetworkStatus::GetAssocAp (const Mac48Address& sta)
{
	NS_LOG_FUNCTION (this);
	Address ap;
	for (auto itr = m_associationMap.begin(); itr != m_associationMap.end(); itr++)
	{
		if (itr->second.find(sta) != itr->second.end())
		{
			ap = itr->first;
			break;
		}
	}
	return ap;
}
} //namespace ns3
