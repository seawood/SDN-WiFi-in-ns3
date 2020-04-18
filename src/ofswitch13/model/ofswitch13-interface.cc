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
 * Author: Luciano Chaves <luciano@lrc.ic.unicamp.br>
 */

#include "ofswitch13-interface.h"
#include "ofswitch13-device.h"
#include "ofswitch13-controller.h"


NS_LOG_COMPONENT_DEFINE ("OFSwitch13Interface");

namespace ns3 {
namespace ofs {

void
EnableLibraryLog (bool printToFile, std::string prefix,
                  bool explicitFilename, std::string customLevels)
{
  set_program_name ("ns3-ofswitch13");
  vlog_init ();
  vlog_set_levels (VLM_ANY_MODULE, VLF_ANY_FACILITY, VLL_EMER);
  vlog_set_pattern (VLF_ANY_FACILITY, "%d{%ss} [%c|%p] %m");

  if (printToFile)
    {
      std::string filename = prefix;
      if (!explicitFilename)
        {
          if (filename.size () && filename.back () != '-')
            {
              filename += "-";
            }
          filename += "ofsoftswitch13.log";
        }
      vlog_set_levels (VLM_ANY_MODULE, VLF_FILE, VLL_DBG);
      vlog_set_log_file (filename.c_str ());
    }
  else
    {
      vlog_set_levels (VLM_ANY_MODULE, VLF_CONSOLE, VLL_DBG);
    }

  if (customLevels.size ())
    {
      vlog_set_verbosity (customLevels.c_str ());
    }
}

struct ofpbuf*
BufferFromPacket (Ptr<const Packet> packet, size_t bodyRoom, size_t headRoom)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT (packet->GetSize () <= bodyRoom);
  struct ofpbuf *buffer;
  uint32_t pktSize;

  pktSize = packet->GetSize ();
  buffer = ofpbuf_new_with_headroom (bodyRoom, headRoom);
  packet->CopyData ((uint8_t*)ofpbuf_put_uninit (buffer, pktSize), pktSize);
  return buffer;
}

Ptr<Packet>
PacketFromMsg (struct ofl_msg_header *msg, uint32_t xid)
{
  NS_LOG_FUNCTION_NOARGS ();

  int error;
  uint8_t *buf;
  size_t buf_size;
  Ptr<Packet> packet;
  struct ofpbuf *buffer;

  buffer = ofpbuf_new (0);
  error = ofl_msg_pack (msg, xid, &buf, &buf_size, &dp_exp); //TODO:dp_exp
  if (!error)
    {
      ofpbuf_use (buffer, buf, buf_size);
      ofpbuf_put_uninit (buffer, buf_size);
      packet = Create<Packet> ((uint8_t*)buffer->data, buffer->size);
      ofpbuf_delete (buffer);
    }
  return packet;
}

Ptr<Packet>
PacketFromBuffer (struct ofpbuf *buffer)
{
  NS_LOG_FUNCTION_NOARGS ();

  return Create<Packet> ((uint8_t*)buffer->data, buffer->size);
}

} // namespace ofs
} // namespace ns3

using namespace ns3;

/**
 * Overriding ofsoftswitch13 time_now weak function from lib/timeval.c.
 * \return The current simulation time, in seconds.
 */
time_t
time_now (void)
{
  return static_cast<time_t> (Simulator::Now ().ToInteger (Time::S));
}

/**
 * Overriding ofsoftswitch13 time_msec weak function from lib/timeval.c.
 * \return The current simulation time, in ms.
 */
long long int
time_msec (void)
{
  return static_cast<long long int> (Simulator::Now ().GetMilliSeconds ());
}

/** Overriding ofsoftswitch weak functions using static member functions. */
void
send_packet_to_controller (struct pipeline *pl, struct packet *pkt,
                           uint8_t table_id, uint8_t reason)
{
  return OFSwitch13Device::SendPacketToController (pl, pkt, table_id, reason);
}

int
send_openflow_buffer_to_remote (struct ofpbuf *buffer, struct remote *remote)
{
  NS_LOG_INFO("send_openflow_buffer_to_remote: overide version");
  return OFSwitch13Device::SendOpenflowBufferToRemote (buffer, remote);
}

void
dp_actions_output_port (struct packet *pkt, uint32_t out_port,
                        uint32_t out_queue, uint16_t max_len, uint64_t cookie)
{
  OFSwitch13Device::DpActionsOutputPort (pkt, out_port, out_queue, max_len,
                                         cookie);
}

void
dpctl_send_and_print (struct vconn *vconn, struct ofl_msg_header *msg)
{
  OFSwitch13Controller::DpctlSendAndPrint (vconn, msg);
}

void
dpctl_transact_and_print (struct vconn *vconn, struct ofl_msg_header *req,
                          struct ofl_msg_header **repl)
{
  // Different from ofsoftswitch13 dpctl, this transaction doesn't wait for a
  // reply, as ns-3 socket library doesn't provide blocking sockets. So, we
  // send the request and return. The reply will came later, using the ns-3
  // callback mechanism.
  OFSwitch13Controller::DpctlSendAndPrint (vconn, req);
}

ofl_err
dp_handle_wifi_config_request(struct datapath *dp, struct ofl_exp_wifi_msg_channel_req *msg,
									  const struct sender *sender)
{
	NS_LOG_DEBUG("dp_handle_wifi_config_request: overide version");
	Ptr<WifiNetDevice> wifiDev = OFSwitch13Device::GetWifiNetDevice(dp->id);
	ofl_err error = 1;
	if(wifiDev)
	{
		struct ofl_exp_wifi_msg_channel reply;
		reply.header.header.header.type = OFPT_EXPERIMENTER;
		reply.header.header.experimenter_id = WIFI_VENDOR_ID;
		reply.header.type = WIFI_EXT_CHANNEL_CONFIG_REPLY;
		reply.channel = (struct ofl_channel_info*)malloc(sizeof(struct ofl_channel_info));
		Ptr<WifiPhy> phy = wifiDev->GetPhy();
		reply.channel->m_channelNumber = phy->GetChannelNumber();
		reply.channel->m_frequency = phy->GetFrequency();
		reply.channel->m_channelWidth = phy->GetChannelWidth();
		Mac48Address::ConvertFrom(wifiDev->GetAddress()).CopyTo(reply.mac48address);
		
		error = dp_send_message(dp, (struct ofl_msg_header*)&reply, sender);
	}
	else
	{
		NS_LOG_ERROR("no wifiNetDevice");
		error = 1;
	}
	return error;
}

ofl_err
dp_handle_wifi_channel_set(struct datapath *dp, struct ofl_exp_wifi_msg_channel *msg,
								   const struct sender *sender)
{
	NS_LOG_DEBUG("dp_handle_wifi_channel_set: overide version");
	Ptr<WifiNetDevice> wifiDev = OFSwitch13Device::GetWifiNetDevice(dp->id);
	ofl_err error = 1;
	if (wifiDev)
	{
		Ptr<WifiPhy> phy = wifiDev->GetPhy();
		phy->SetChannelNumber(msg->channel->m_channelNumber);
		phy->SetFrequency(msg->channel->m_frequency);
		phy->SetChannelWidth(msg->channel->m_channelWidth);
		error = 0;
	}
	else
	{
		NS_LOG_ERROR("no wifiNetDevice");
		error = 1;
	}
	return error;
	
}

ofl_err
dp_handle_wifi_chanqua_request(struct datapath *dp,
							   struct ofl_exp_wifi_msg_chaqua_req *msg,
							   const struct sender *sender)
{
	NS_LOG_DEBUG ("dp_handle_wifi_chanqua_request: overide version");
	Ptr<WifiNetDevice> wifiDev = OFSwitch13Device::GetWifiNetDevice(dp->id);
	ofl_err error = 1;
	if (wifiDev)
	{
		struct ofl_exp_wifi_msg_chaqua reply;
		reply.header.header.header.type = OFPT_EXPERIMENTER;
		reply.header.header.experimenter_id = WIFI_VENDOR_ID;
		reply.header.type = WIFI_EXT_CHANNEL_QUALITY_REPLY;
		Mac48Address requestAddr;
		requestAddr.CopyFrom (msg->mac48address);
		Ptr<SpectrumWifiPhy> phy = DynamicCast<SpectrumWifiPhy, WifiPhy>(wifiDev->GetPhy());
		SpectrumWifiPhy::ChannelQualityMap* record = phy->GetChannelQualityRecord();
		if (requestAddr.IsBroadcast()) // request all channel quality info
		{
			reply.num = record->size();
			reply.reports = (struct chaqua_report**)malloc(reply.num * sizeof(struct chaqua_report*));
			int i = 0;
			for (auto itr = record->begin(); itr != record->end(); ++itr)
			{
				reply.reports[i] = (struct chaqua_report*)malloc(sizeof(struct chaqua_report));
				itr->first.CopyTo(reply.reports[i]->mac48address);
				reply.reports[i]->packets = itr->second.packets;
				reply.reports[i]->rxPower_avg = itr->second.rxPower_avg;
				reply.reports[i]->rxPower_std = itr->second.rxPower_std;
				++i;
			}
		}
		else  // request channel quality info of a specific STA
		{
			reply.num = 1;
			reply.reports = (struct chaqua_report**)malloc(sizeof(struct chaqua_report*));
			for (auto itr = record->begin(); itr != record->end(); ++itr)
			{
				if (requestAddr == itr->first)
				{
					reply.reports[0] = (struct chaqua_report*) malloc(sizeof(struct chaqua_report));
					requestAddr.CopyTo(reply.reports[0]->mac48address);
					reply.reports[0]->packets = itr->second.packets;
					reply.reports[0]->rxPower_avg = itr->second.rxPower_avg;
					reply.reports[0]->rxPower_std = itr->second.rxPower_std;
					break;
				}
			}
		}
		error = dp_send_message(dp, (struct ofl_msg_header*)&reply, sender);
	}
	else
	{
		NS_LOG_ERROR("no wifiNetDevice");
		error = 1;
	}
	return error;
}

ofl_err
dp_handle_wifi_chanqua_trigger_set (struct datapath *dp,
									struct ofl_exp_wifi_msg_chaqua *msg,
									const struct sender *sender)
{
	NS_LOG_DEBUG ("dp_handle_wifi_chanqua_trigger_set: overide version");
	Ptr<WifiNetDevice> wifiDev = OFSwitch13Device::GetWifiNetDevice(dp->id);
	ofl_err error = 1;
	if (wifiDev)
	{
		Ptr<SpectrumWifiPhy> phy = DynamicCast<SpectrumWifiPhy, WifiPhy>(wifiDev->GetPhy());
		SpectrumWifiPhy::ChannelQualityMap* record = phy->GetChannelQualityRecord();
		for (uint32_t i = 0; i < msg->num; ++i)
		{
			Mac48Address setAddr;
			setAddr.CopyFrom (msg->reports[i]->mac48address);
			auto item = record->find (setAddr);
			if (item != record->end())
			{
				item->second.trigger_set = true;
				item->second.packets_trigger = msg->reports[i]->packets;
				item->second.rxPower_avg_trigger = msg->reports[i]->rxPower_avg;
				item->second.rxPower_std_trigger = msg->reports[i]->rxPower_std;
				error = 0;
			}
			else
			{
				error = 1;
				NS_LOG_ERROR("set trigger for non existing device");
				break;
			}
		}
	}
	else
	{
		NS_LOG_ERROR("no wifiNetDevice");
		error = 1;
	}
	return error;
}

ofl_err dp_handle_wifi_assoc_status_request (struct datapath *dp, 
											struct ofl_exp_wifi_msg_channel_re *msg, 
											const struct sender *sender)
{
	NS_LOG_DEBUG ("dp_handle_wifi_assoc_status_request: overide version");
	Ptr<WifiNetDevice> wifiDev = OFSwitch13Device::GetWifiNetDevice(dp->id);
	ofl_err error = 1;
	if (wifiDev)
	{
		Ptr<ApWifiMac> mac = DynamicCast<ApWifiMac, WifiMac>(wifiDev.GetMac());
		std::vector<Mac48Address> stas;
		mac->GetStas(stas);
		mac->SetAssocTrigger();
		struct ofl_ext_wifi_msg_assoc reply;
		reply.header.header.header.type = OFPT_EXPERIMENTER;
		reply.header.header.experimenter_id = WIFI_VENDOR_ID;
		reply.header.type = WIFI_EXT_ASSOC_STATUS_REPLY;
		reply.num = stas.size();
		reply.reports = (struct sta_address**)malloc(reply.num * sizeof(struct sta_address*));
		for (int i = 0; i <reply.num; ++i)
		{
			reply.reports[i] = (struct sta_address*)malloc(sizeof(struct sta_address));
			stas.CopyTo(reply.reports[i]->mac48address);
		}
		error = dp_send_message(dp, (struct ofl_msg_header*)&reply, sender);
	}
	return error;
}


ofl_err 
dp_handle_wifi_disassoc_config (struct datapath *dp, 
								struct ofl_ext_wifi_msg_assoc *msg, 
								const struct sender *sender)
{
	NS_LOG_DEBUG ("dp_handle_wifi_disassoc_config: overide version");
	Ptr<WifiNetDevice> wifiDev = OFSwitch13Device::GetWifiNetDevice(dp->id);
	
	ofl_err error = 1;
	if (wifiDev)
	{
		Mac48Address sta;
		sta.CopyFrom (msg->addresses.mac48address);
		Ptr<ApWifiMac> mac = DynamicCast<ApWifiMac, WifiMac>(wifiDev.GetMac());
		Buffer mgtHeader = mac->GetMgtHeader(sta);
		mac->DisassocSTA (sta);
		struct ofl_ext_wifi_msg_assoc reply;
		reply.header.header.header.type = OFPT_EXPERIMENTER;
		reply.header.header.experimenter_id = WIFI_VENDOR_ID;
		reply.header.type = ofl_ext_wifi_msg_assoc_disassoc_config;
		sta.CopyTo(reply.mac48address);
		reply.len = mgtHeader.GetSerializedSize();
		uint8_t *data = (uint8_t*)malloc(reply.len);
		mgtHeader.Serialize(data, reply.len);
		reply.data = (uint8_t**)malloc(sizeof(uint8_t*));
		reply.data[0] = data;
		error = dp_send_message(dp, (struct ofl_msg_header*)&reply, sender);
	}
	return error;
}

ofl_err 
dp_handle_wifi_assoc_config (struct datapath *dp, 
							 struct ofl_ext_wifi_msg_assoc_disassoc_config *msg, 
							 const struct sender *sender) {
	NS_LOG_DEBUG ("dp_handle_wifi_assoc_config: overide version");
	Ptr<WifiNetDevice> wifiDev = OFSwitch13Device::GetWifiNetDevice(dp->id);
	ofl_err error = 1;
	if (wifiDev)
	{
		Mac48Address sta;
		sta.CopyFrom (msg->addresses.mac48address);
		Ptr<ApWifiMac> mac = DynamicCast<ApWifiMac, WifiMac>(wifiDev.GetMac());
		Buffer mgtHeader(msg->len);
		mgtHeader.Deserialize(msg->data,msg->len);
		error = mac->AssocSTA(sta, mgtHeader);
	}
	return error;
}
