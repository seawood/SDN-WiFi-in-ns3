/* Copyright (c) 2020, Peking University
 * Author: xie yingying <xyynku@163.com>
**/

#ifndef OFL_EXP_WIFI_H
#define OFL_EXP_WIFI_H 1

#include "../oflib/ofl-messages.h"


struct ofl_exp_wifi_msg_header {
    struct ofl_msg_experimenter   header; /* WIFI_VENDOR_ID */

    uint32_t   type;
};

struct ofl_channel_info {
	uint8_t m_channelNumber;
	uint16_t m_frequency;
	uint16_t m_channelWidth;
};

//WIFI_EXT_CHANNEL_CONFIG_REQUEST,
struct ofl_exp_wifi_msg_channel_req {
	struct ofl_exp_wifi_msg_header header;
};

//WIFI_EXT_CHANNEL_CONFIG_REPLY,WIFI_EXT_CHANNEL_SET
struct ofl_exp_wifi_msg_channel {  
	struct ofl_exp_wifi_msg_header header;
	struct ofl_channel_info *channel;
	uint8_t mac48address[6];
};

int
ofl_exp_wifi_msg_pack(struct ofl_msg_experimenter *msg, uint8_t **buf, size_t *buf_len);

ofl_err
ofl_exp_wifi_msg_unpack(struct ofp_header *oh, size_t *len, struct ofl_msg_experimenter **msg);

int
ofl_exp_wifi_msg_free(struct ofl_msg_experimenter *msg);

char *
ofl_exp_wifi_msg_to_string(struct ofl_msg_experimenter *msg);

#endif  /*OFL_EXP_WIFI_H*/
