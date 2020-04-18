/*
 * Copyright (c) 2020 Peking University
 * Author: xie yingying <xyynku@163.com>
 */

#ifndef OPENFLOW_WIFI_EXT_H
#define OPENFLOW_WIFI_EXT_H 1

#include "openflow/openflow.h"

#define WIFI_VENDOR_ID 0x80000001

enum wifi_extension_subtype {
	WIFI_EXT_CHANNEL_CONFIG_REQUEST,
	WIFI_EXT_CHANNEL_CONFIG_REPLY,
	WIFI_EXT_CHANNEL_SET,
	WIFI_EXT_CHANNEL_QUALITY_REQUEST,
	WIFI_EXT_CHANNEL_QUALITY_REPLY,
	WIFI_EXT_CHANNEL_QUALITY_TRIGGER_SET,
	WIFI_EXT_CHANNEL_QUALITY_TRIGGERED,
	WIFI_EXT_ASSOC_STATUS_REQUEST,
	WIFI_EXT_ASSOC_STATUS_REPLY,
	WIFI_EXT_ASSOC_TRIGGERRED,
	WIFI_EXT_DIASSOC_TRIGGERED,
	WIFI_EXT_DISASSOC_CONFIG,
	WIFI_EXT_DISASSOC_CONFIG_REPLY,
	WIFI_EXT_ASSOC_CONFIG
};

//WIFI_EXT_CHANNEL_CONFIG_REQUEST
//WIFI_EXT_ASSOC_STATUS_REQUEST
struct wifi_extension_header {
    struct ofp_header header;
    uint32_t vendor;            /* OPENFLOW_VENDOR_ID. */
    uint32_t subtype;           /* One of wifi_extension_subtype */
};
OFP_ASSERT(sizeof(struct wifi_extension_header) == 16);

//WIFI_EXT_CHANNEL_CONFIG_REPLY,
//WIFI_EXT_CHANNEL_SET
struct wifi_channel_header {
	struct wifi_extension_header header;
	uint16_t m_frequency;
	uint16_t m_channelWidth;
	uint8_t m_channelNumber;
	uint8_t pad1[3];    /* Align to 64-bits */
	uint8_t m_mac48address[6];
	uint8_t pad2[2];
};
OFP_ASSERT(sizeof(struct wifi_channel_header) == 32);

//WIFI_EXT_CHANNEL_QUALITY_REQUEST
struct wifi_channel_quality_request {
	struct wifi_extension_header header;
	uint8_t mac48address[6];  //0 for all
	uint8_t pad[2];
};
OFP_ASSERT(sizeof(struct wifi_channel_quality_request) == 24);

struct channel_quality_report {
	uint8_t mac48address[6];
	uint8_t pad[2];
	uint64_t packets;     //number of received packets
	double rxPower_avg; //average ?double
	double rxPower_std; //standard deviation
};
OFP_ASSERT(sizeof(struct channel_quality_report) == 32);

//WIFI_EXT_CHANNEL_QUALITY_REPLY,
//WIFI_EXT_CHANNEL_QUALITY_TRIGGER_SET,
//WIFI_EXT_CHANNEL_QUALITY_TRIGGERED
struct wifi_channel_quality {
	struct wifi_extension_header header;
	uint32_t num;
	uint8_t pad[4];
	struct channel_quality_report reports[0]; //list of channel quality report
};
OFP_ASSERT(sizeof(struct wifi_channel_quality) == 24);


struct assoc_status {
	uint8_t mac48address[6];
	uint8_t pad[2];
};
OFP_ASSERT(sizeof(struct assoc_status) == 8);
//WIFI_EXT_ASSOC_STATUS_REPLY
//WIFI_EXT_ASSOC_TRIGGERRED,
//WIFI_EXT_DIASSOC_TRIGGERED,
//WIFI_EXT_DISASSOC_CONFIG
struct wifi_assoc_status {
	struct wifi_extension_header header;
	uint32_t num;
	uint8_t pad[4];
	struct assoc_status status[0];
};
OFP_ASSERT(sizeof(struct wifi_assoc_status) == 24);


//WIFI_EXT_DISASSOC_CONFIG_REPLY,
//WIFI_EXT_ASSOC_CONFIG
struct wifi_assoc_disassoc_config {
	struct wifi_extension_header header;
	uint32_t len;
	uint8_t pad[4];
	uint8_t mac48address[6];
	uint8_t pad[2];
	uint8_t data[0];
};
OFP_ASSERT(sizeof(struct wifi_assoc_disassoc_config) == 32);

#endif  /*OPENFLOW_WIFI_EXT_H*/

