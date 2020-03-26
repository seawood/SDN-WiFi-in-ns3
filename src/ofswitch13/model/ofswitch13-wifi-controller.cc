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
#include "ofswitch13-controller.h"
#include "openflow/wifi-ext.h"
#include "oflib-exp/ofl-exp-wifi.h"

NS_LOG_COMPONENT_DEFINE ("OFSwitch13WifiController");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (OFSwitch13WifiController);

OFSwitch13WifiController::OFSwitch13WifiController()
{
	NS_LOG_FUNCTION (this);
}

OFSwitch13WifiController::~OFSwitch13WifiController()
{
	NS_LOG_FUNCTION (this);
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
			Ptr<WifiAp> ap = GetWifiAp (swtch->GetAddress());
			ap->SetChannelInfo (exp->channel->m_channelNumber, exp->channel->m_frequency,
								exp->channel->m_channelWidth);
			GetWifiNetworkStatus()->UpdateFrequencyUsed (swtch->GetAddress(), 
					exp->channel->m_frequency, exp->channel->m_channelWidth);
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

void 
OFSwitch13WifiController::ConfiChannelStrategy (void)
{
	//TODO: channel allocation algorithm
	Ptr<WifiApsMap_t> apsMap = OFSwitch13Controller::GetWifiApsMap();
	for (auto it = apsMap->begin(); it != apsMap->end(); ++it)
	{
		ConfigChannel (it->first, 13, 2470, 20);
	}
}

void
OFSwitch13WifiController::ConfigChannel (const Address& address, const uint8_t& channelNumber,
					const uint16_t frequency, const uint16_t& channelWidth)
{
	Ptr<RemoteSwitch> swtch = GetRemoteSwitch (address);
	Ptr<WifiAp> ap = GetWifiAp (address);
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
	GetWifiNetworkStatus()->UpdateFrequencyUsed (address, frequency, channelWidth);
}

} // namespace ns3

#endif  //NS3_OFSWITCH13
