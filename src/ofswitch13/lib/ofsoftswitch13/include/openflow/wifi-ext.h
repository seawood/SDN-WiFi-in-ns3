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
	WIFI_EXT_CHANNEL_SET
};

struct wifi_extension_header {
    struct ofp_header header;
    uint32_t vendor;            /* OPENFLOW_VENDOR_ID. */
    uint32_t subtype;           /* One of wifi_extension_subtype */
};
OFP_ASSERT(sizeof(struct wifi_extension_header) == 16);

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

#endif  /*OPENFLOW_WIFI_EXT_H*/

