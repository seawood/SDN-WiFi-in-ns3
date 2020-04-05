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

#ifdef NS3_OFSWITCH13

#include "ofswitch13-wifi-controller.h"

NS_LOG_COMPONENT_DEFINE ("OFSwitch13WifiController");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (OFSwitch13WifiController);

OFSwitch13WifiController::OFSwitch13WifiController()
{
	NS_LOG_FUNCTION (this);
	m_wifiNetworkStatus = Create<WifiNetworkStatus>();
}

OFSwitch13WifiController::~OFSwitch13WifiController()
{
	NS_LOG_FUNCTION (this);
	m_wifiNetworkStatus->PrintChannelQuality();
}

TypeId
OFSwitch13WifiController::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::OFSwitch13WifiController")
						.SetParent<OFSwitch13Controller> ()
						.SetGroupName ("OFSwitch13")
						.AddConstructor<OFSwitch13WifiController> ();
	return tid;
}

void
OFSwitch13WifiController::DoDispose ()
{
	NS_LOG_FUNCTION (this);
	OFSwitch13LearningController::DoDispose ();
}

ofl_err
OFSwitch13WifiController::HandleExperimenterMsg (
	struct ofl_exp_wifi_msg_header *msg, Ptr<const RemoteSwitch> swtch,
	uint32_t xid)
{
	NS_LOG_FUNCTION (this);
	if (msg->header.experimenter_id != WIFI_VENDOR_ID)
	{
		NS_LOG_ERROR ("unable to handle non-wifi experimenter msg");
		return 1;
	}
	switch (msg->type)
	{
		case (WIFI_EXT_CHANNEL_CONFIG_REPLY):
		{
			struct ofl_exp_wifi_msg_channel* exp = (struct ofl_exp_wifi_msg_channel*)msg;
			Ptr<WifiAp> ap = m_wifiApsMap[swtch->GetAddress()];
			ap->SetMac48Address (exp->mac48address);
			Mac48Address addr = ap->GetMac48Address();
			NS_LOG_INFO ("SetMac48Address=" << addr);
			m_wifiNetworkStatus->AddApMac48address (ap->GetMac48Address());
			m_wifiApsMac48Map[ap->GetMac48Address()] = ap;
			ap->SetChannelInfo (exp->channel->m_channelNumber, exp->channel->m_frequency,
								exp->channel->m_channelWidth);
			m_wifiNetworkStatus->UpdateFrequencyUsed (swtch->GetAddress(), 
					exp->channel->m_frequency, exp->channel->m_channelWidth);
			ofl_msg_free((struct ofl_msg_header*)msg, &dp_exp);
			break;
		}
		case (WIFI_EXT_CHANNEL_QUALITY_TRIGGERED): // TODO: trigger handle
		case (WIFI_EXT_CHANNEL_QUALITY_REPLY):
		{
			struct ofl_exp_wifi_msg_chaqua* exp = (struct ofl_exp_wifi_msg_chaqua*)msg;
			Ptr<WifiAp> ap = m_wifiApsMap[swtch->GetAddress()];
			Address apAddr = swtch->GetAddress();
			for (uint64_t i = 0; i < exp->num; ++i)
			{
				Mac48Address addr;
				addr.CopyFrom (exp->reports[i]->mac48address);
				auto item = m_wifiApsMac48Map.find(addr);
				if (item != m_wifiApsMac48Map.end()) //AP
				{
					m_wifiNetworkStatus->UpdateApsInterference (apAddr, 
							m_wifiApsMac48Map[addr]->GetAddress(), exp->reports[i]);
				}
				else
				{
					m_wifiNetworkStatus->UpdateChannelQuality(apAddr, exp->reports[i]);
				}
			}
			ofl_msg_free((struct ofl_msg_header*)msg, &dp_exp);
			break;
		}
		default:
		{
			NS_LOG_ERROR ("unable to handle experimenter msg : unsupported type");
			return 1;
		}
	}
	
	return 0;
}

ofl_err
OFSwitch13WifiController::HandleFeaturesReplyWifi (Ptr<const RemoteSwitch> swtch)
{
	NS_LOG_FUNCTION (this);
	// update m_wifiApsMap
	Ptr<WifiAp> ap = Create<WifiAp> (swtch->GetAddress());
	m_wifiApsMap.insert (std::make_pair (swtch->GetAddress(), ap));
	// send experimenter msg to query for initial channel configuration
	struct ofl_exp_wifi_msg_header msg;
	msg.header.header.type = OFPT_EXPERIMENTER;
	msg.header.experimenter_id = WIFI_VENDOR_ID;
	msg.type = WIFI_EXT_CHANNEL_CONFIG_REQUEST;
	ofl_err err = SendToSwitch (swtch, (struct ofl_msg_header*)&msg);
	NS_LOG_DEBUG ("send WIFI_EXT_CHANNEL_CONFIG_REQUEST to wifi ap");
	return err;
}

void 
OFSwitch13WifiController::ConfigChannelStrategy (void)
{
	//TODO (xyy): channel allocation algorithm
	NS_LOG_FUNCTION (this << "switch all to channel 13");
	NS_LOG_DEBUG ("m_wifiApsMap size: " << m_wifiApsMap.size());
	for (auto it = m_wifiApsMap.begin(); it != m_wifiApsMap.end(); ++it)
	{
		ConfigChannel (it->first, 13, 2472, 20);
	}
}

// request all
void
OFSwitch13WifiController::ChannelQualityReportStrategy (void)
{
	NS_LOG_FUNCTION (this);
	Mac48Address all("ff:ff:ff:ff:ff:ff");
	for (auto itr = m_wifiApsMap.begin(); itr != m_wifiApsMap.end(); ++itr)
	{
		RequestChannelQuality (itr->first, all);
	}
}


void
OFSwitch13WifiController::ChannelQualityTriggerStrategy (void)
{
	NS_LOG_FUNCTION (this);
	Address ap;
	Mac48Address sta;
	m_wifiNetworkStatus->GetOneSTA(&ap, &sta);
	std::vector<struct OneReport> triggers;
	struct OneReport trigger;
	trigger.address = sta;
	trigger.packets = 10;
	trigger.rxPower_avg = 10;
	trigger.rxPower_std = 10;
	triggers.push_back(trigger);
	SetChannelQualityTrigger(ap, triggers);
}

void
OFSwitch13WifiController::ConfigChannel (const Address& address, const uint8_t& channelNumber,
					const uint16_t frequency, const uint16_t& channelWidth)
{
	NS_LOG_FUNCTION (this);
	Ptr<RemoteSwitch> swtch = GetRemoteSwitch (address);
	Ptr<WifiAp> ap = m_wifiApsMap[address];
	ap->SetChannelInfo (channelNumber, frequency, channelWidth);
	struct ofl_exp_wifi_msg_channel msg;
	msg.header.header.header.type = OFPT_EXPERIMENTER;
	msg.header.header.experimenter_id = WIFI_VENDOR_ID;
	msg.header.type = WIFI_EXT_CHANNEL_SET;
	struct ofl_channel_info info;
	info.m_channelNumber = channelNumber;
	info.m_frequency = frequency;
	info.m_channelWidth = channelWidth;
	msg.channel = &info;
	
	SendToSwitch (swtch, (struct ofl_msg_header*)&msg);
	NS_LOG_DEBUG ("sent WIFI_EXT_CHANNEL_SET to wifi ap");
	m_wifiNetworkStatus->UpdateFrequencyUsed (address, frequency, channelWidth);
}

void
OFSwitch13WifiController::RequestChannelQuality (const Address& address, 
		const Mac48Address& mac48address)
{
	NS_LOG_FUNCTION (this);
	Ptr<RemoteSwitch> swtch = GetRemoteSwitch (address);
	struct ofl_exp_wifi_msg_chaqua_req msg;
	msg.header.header.header.type = OFPT_EXPERIMENTER;
	msg.header.header.experimenter_id = WIFI_VENDOR_ID;
	msg.header.type = WIFI_EXT_CHANNEL_QUALITY_REQUEST;
	mac48address.CopyTo (msg.mac48address);
	
	SendToSwitch (swtch, (struct ofl_msg_header*)&msg);
	NS_LOG_DEBUG ("sent WIFI_EXT_CHANNEL_QUALITY_REQUEST to wifi ap");
}

void
OFSwitch13WifiController::SetChannelQualityTrigger (const Address& address, 
		const std::vector<struct OneReport>& triggers)
{
	NS_LOG_FUNCTION (this);
	Ptr<RemoteSwitch> swtch = GetRemoteSwitch (address);
	struct ofl_exp_wifi_msg_chaqua msg;
	msg.header.header.header.type = OFPT_EXPERIMENTER;
	msg.header.header.experimenter_id = WIFI_VENDOR_ID;
	msg.header.type = WIFI_EXT_CHANNEL_QUALITY_TRIGGER_SET;
	msg.num = triggers.size();
	msg.reports = (struct chaqua_report**)malloc(msg.num * sizeof(struct chaqua_report*));
	for (uint32_t i = 0; i < msg.num; ++i)
	{
		msg.reports[i] = (struct chaqua_report*)malloc(sizeof(struct chaqua_report));
		triggers[i].address.CopyTo(msg.reports[i]->mac48address);
		msg.reports[i]->packets = triggers[i].packets;
		msg.reports[i]->rxPower_avg = triggers[i].rxPower_avg;
		msg.reports[i]->rxPower_std = triggers[i].rxPower_std;
	}
	
	SendToSwitch (swtch, (struct ofl_msg_header*)&msg);
	NS_LOG_DEBUG ("sent WIFI_EXT_CHANNEL_QUALITY_TRIGGER_SET to wifi ap");
}

} // namespace ns3

#endif  //NS3_OFSWITCH13
