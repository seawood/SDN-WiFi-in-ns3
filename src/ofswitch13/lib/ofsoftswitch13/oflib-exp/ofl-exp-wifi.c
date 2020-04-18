/* Copyright (c) 2020, Peking University
 * Author: xie yingying <xyynku@163.com>
**/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "openflow/openflow.h"
#include "openflow/wifi-ext.h"
#include "ofl-exp-wifi.h"
#include "../oflib/ofl-log.h"
#include "../oflib/ofl-print.h"

#define LOG_MODULE ofl_exp_wifi
OFL_LOG_INIT(LOG_MODULE)

int
ofl_exp_wifi_msg_pack(struct ofl_msg_experimenter *msg, 
					  uint8_t **buf, size_t *buf_len)
{
	if (msg->experimenter_id == WIFI_VENDOR_ID) {
		struct ofl_exp_wifi_msg_header *exp = (struct ofl_exp_wifi_msg_header*)msg;
		switch (exp->type){
			case (WIFI_EXT_CHANNEL_CONFIG_REQUEST): 
			case (WIFI_EXT_ASSOC_STATUS_REQUEST): {
				struct wifi_extension_header* wifi;
				*buf_len = sizeof(struct wifi_extension_header);
				*buf = (uint8_t*)calloc(1, *buf_len);
				wifi = (struct wifi_extension_header*)(*buf);
				wifi->vendor = htonl(exp->header.experimenter_id);
				wifi->subtype = htonl(exp->type);
				break;
			}
			case (WIFI_EXT_CHANNEL_CONFIG_REPLY):
			case (WIFI_EXT_CHANNEL_SET): {
				struct ofl_exp_wifi_msg_channel* c = (struct ofl_exp_wifi_msg_channel*)exp;
				struct wifi_channel_header* wifi;
				*buf_len = sizeof(struct wifi_channel_header);
				*buf = (uint8_t*)calloc(1, *buf_len);
				wifi = (struct wifi_channel_header*)(*buf);
				wifi->header.vendor = htonl(exp->header.experimenter_id);
				wifi->header.subtype = htonl(exp->type);
				wifi->m_channelNumber = htonl(c->channel->m_channelNumber);
				wifi->m_frequency = htons(c->channel->m_frequency);
				wifi->m_channelWidth = htons(c->channel->m_channelWidth);
				memcpy (wifi->m_mac48address, c->mac48address, 6); //hton?
				break;
			}
			case (WIFI_EXT_CHANNEL_QUALITY_REQUEST): {
				struct ofl_exp_wifi_msg_chaqua_req* src = (struct ofl_exp_wifi_msg_chaqua_req*)exp;
				struct wifi_channel_quality_request* dst;
				*buf_len = sizeof(struct wifi_channel_quality_request);
				*buf = (uint8_t*)malloc(*buf_len);
				dst = (struct wifi_channel_quality_request*)(*buf);
				dst->header.vendor = htonl(exp->header.experimenter_id);
				dst->header.subtype = htonl(exp->type);
				memcpy (dst->mac48address, src->mac48address, 6);
				break;
			}
			case (WIFI_EXT_CHANNEL_QUALITY_REPLY):
			case (WIFI_EXT_CHANNEL_QUALITY_TRIGGER_SET):
			case (WIFI_EXT_CHANNEL_QUALITY_TRIGGERED): {
				struct ofl_exp_wifi_msg_chaqua* src = (struct ofl_exp_wifi_msg_chaqua*)exp;
				struct wifi_channel_quality* dst;
				*buf_len = sizeof(struct wifi_channel_quality) + src->num * sizeof(struct channel_quality_report);
				*buf = (uint8_t*)malloc (*buf_len);
				dst = (struct wifi_channel_quality*)(*buf);
				dst->header.vendor = htonl(exp->header.experimenter_id);
				dst->header.subtype = htonl(exp->type);
				dst->num = htonl(src->num);
				for (size_t i = 0; i <src->num; ++i)
				{
					memcpy (dst->reports[i].mac48address, src->reports[i]->mac48address, 6);
					dst->reports[i].packets = htonl(src->reports[i]->packets);
					dst->reports[i].rxPower_avg = htonl(src->reports[i]->rxPower_avg);
					dst->reports[i].rxPower_std = htonl(src->reports[i]->rxPower_std);
				}
				break;
			}
			case (WIFI_EXT_ASSOC_STATUS_REPLY):
			case (WIFI_EXT_ASSOC_TRIGGERRED):
			case (WIFI_EXT_DIASSOC_TRIGGERED):
			case (WIFI_EXT_DISASSOC_CONFIG): {
				struct ofl_ext_wifi_msg_assoc* src = (struct ofl_ext_wifi_msg_assoc*)exp;
				struct wifi_assoc_status* dst;
				*buf_len = sizeof(struct wifi_assoc_status) + src->num * sizeof(struct assoc_status);
				*buf = (uint8_t*)malloc(*buf_len);
				dst = (struct wifi_assoc_status*)(*buf);
				dst->header.vendor = htonl(exp->header.experimenter_id);
				dst->header.subtype = htonl(exp->type);
				dst->num = htonl(src->num);
				for(size_t i = 0; i < src->num; ++i)
				{
					memcpy(dst->status[i].mac48address, src->addresses[i]->mac48address, 6);
				}
				break;
			}
			case (WIFI_EXT_DISASSOC_CONFIG_REPLY):
			case (WIFI_EXT_ASSOC_CONFIG): {
				struct ofl_ext_wifi_msg_assoc_disassoc_config* src = (struct ofl_ext_wifi_msg_assoc_disassoc_config*)exp;
				struct wifi_assoc_disassoc_config* dst;
				*buf_len = sizeof(struct wifi_assoc_disassoc_config) + src->len;
				*buf = (struct wifi_assoc_disassoc_config*)malloc(*buf_len);
				dst = (struct wifi_assoc_disassoc_config*)(*buf);
				dst->header.vendor = htonl(exp->header.experimenter_id);
				dst->header.subtype = htonl(exp->type);
				dst->len = htonl(src->len);
				memcpy(dst->mac48address, src->mac48address, 6);
				memcpy(dst->data, src->data, src->len);
				break;
			}
			default: {
				OFL_LOG_WARN(LOG_MODULE, "Trying to pack unknown Openflow Experimenter message.");
                return -1;
			}
		}
				
	}else {
		OFL_LOG_WARN(LOG_MODULE, "Trying to pack non-wifi Experimenter message.");
        return -1;
	}
	return 0;
}

ofl_err
ofl_exp_wifi_msg_unpack(struct ofp_header *oh, size_t *len,
						struct ofl_msg_experimenter **msg)
{
	struct wifi_extension_header* exp;
	if (*len < sizeof(struct wifi_extension_header)) {
		OFL_LOG_WARN(LOG_MODULE, "Received wifi EXPERIMENTER message has invalid length (%zu).", *len);
        return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
	}
	
	exp = (struct wifi_extension_header*)oh;
	OFL_LOG_DBG(LOG_MODULE, "exp->vendor:: %x", ntohl(exp->vendor));
	if (ntohl(exp->vendor) == WIFI_VENDOR_ID) {
		switch (ntohl(exp->subtype)) {
			case (WIFI_EXT_CHANNEL_CONFIG_REQUEST):
			case (WIFI_EXT_ASSOC_STATUS_REQUEST): {
				struct ofl_exp_wifi_msg_channel_req* dst;
				dst = (struct ofl_exp_wifi_msg_channel_req*)malloc(sizeof(struct ofl_exp_wifi_msg_channel_req));
				dst->header.header.experimenter_id = ntohl(exp->vendor);
				dst->header.type = ntohl(exp->subtype);
				(*msg) = (struct ofl_msg_experimenter*)dst;
				*len -= sizeof(struct wifi_extension_header);
				OFL_LOG_DBG(LOG_MODULE, "unpack WIFI_EXT_CHANNEL_CONFIG");
				return 0;
			}
			case (WIFI_EXT_CHANNEL_CONFIG_REPLY):
			case (WIFI_EXT_CHANNEL_SET): {
				struct wifi_channel_header* src;
				struct ofl_exp_wifi_msg_channel* dst;
				if (*len < sizeof(struct wifi_channel_header))
				{
					OFL_LOG_WARN(LOG_MODULE, "Received  message WIFI_EXT_CHANNEL_CONFIG_REPLY/WIFI_EXT_CHANNEL_SET has invalid length (%zu).", *len);
                    return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
				}
				OFL_LOG_WARN(LOG_MODULE, "unpack WIFI_EXT_CHANNEL or CONFIG");
				src = (struct wifi_channel_header*)exp;
				dst = (struct ofl_exp_wifi_msg_channel*)malloc(sizeof(struct ofl_exp_wifi_msg_channel));
				dst->header.header.experimenter_id = ntohl(exp->vendor);
				dst->header.type = ntohl(exp->subtype);
				dst->channel = (struct ofl_channel_info*)malloc(sizeof(struct ofl_channel_info));
				dst->channel->m_channelNumber = ntohs(src->m_channelNumber);
				dst->channel->m_frequency = ntohs(src->m_frequency);
				dst->channel->m_channelWidth = ntohs(src->m_channelWidth);
				memcpy (dst->mac48address, src->m_mac48address, 6);
				(*msg) = (struct ofl_msg_experimenter*)dst;
				*len -= sizeof(struct wifi_channel_header);
				return 0;	
			}
			case (WIFI_EXT_CHANNEL_QUALITY_REQUEST): {
				struct wifi_channel_quality_request* src;
				struct ofl_exp_wifi_msg_chaqua_req* dst;
				if (*len < sizeof(struct wifi_channel_quality_request))
				{
					OFL_LOG_WARN(LOG_MODULE, "Received  message WIFI_EXT_CHANNEL_QUALITY_REQUEST has invalid length (%zu).", *len);
                    return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
				}
				OFL_LOG_WARN(LOG_MODULE, "unpack WIFI_EXT_CHANNEL_QUALITY_REQUEST");
				src = (struct wifi_channel_quality_request*)exp;
				dst = (struct ofl_exp_wifi_msg_chaqua_req*)malloc(sizeof(struct ofl_exp_wifi_msg_chaqua_req));
				dst->header.header.experimenter_id = ntohl(exp->vendor);
				dst->header.type = ntohl(exp->subtype);
				memcpy (dst->mac48address, src->mac48address, 6);
				(*msg) = (struct ofl_msg_experimenter*)dst;
				*len -= sizeof(struct wifi_channel_quality_request);
				return 0;
			}
			case (WIFI_EXT_CHANNEL_QUALITY_REPLY):
			case (WIFI_EXT_CHANNEL_QUALITY_TRIGGER_SET):
			case (WIFI_EXT_CHANNEL_QUALITY_TRIGGERED): {
				struct wifi_channel_quality* src;
				struct ofl_exp_wifi_msg_chaqua* dst;
				if (*len < sizeof(struct wifi_channel_quality))
				{
					OFL_LOG_WARN(LOG_MODULE, "Received  message WIFI_EXT_CHANNEL_QUALITY_ has invalid length (%zu).", *len);
                    return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
				}
				OFL_LOG_WARN(LOG_MODULE, "unpack WIFI_EXT_CHANNEL_QUALITY_");
				src = (struct wifi_channel_quality*)exp;
				dst = (struct ofl_exp_wifi_msg_chaqua*)malloc(sizeof(struct ofl_exp_wifi_msg_chaqua));
				dst->header.header.experimenter_id = ntohl(exp->vendor);
				OFL_LOG_DBG(LOG_MODULE, "dst->header.header.experimenter_id : %x", dst->header.header.experimenter_id );
				dst->header.type = ntohl(exp->subtype);
				dst->num = ntohl (src->num);
				OFL_LOG_DBG(LOG_MODULE, "dst->num: %u", dst->num);
				dst->reports = (struct chaqua_report**)malloc(dst->num * sizeof(struct chaqua_report*));
				for (size_t i = 0; i < dst->num; ++i)
				{
					dst->reports[i] = (struct chaqua_report*)malloc(sizeof(struct chaqua_report));
					memcpy (dst->reports[i]->mac48address, src->reports[i].mac48address,  6);
					dst->reports[i]->packets = ntohl(src->reports[i].packets);
					dst->reports[i]->rxPower_avg = ntohl(src->reports[i].rxPower_avg);
					dst->reports[i]->rxPower_std = ntohl(src->reports[i].rxPower_std);
				}
				(*msg) = (struct ofl_msg_experimenter*)dst;
				*len -= sizeof(struct wifi_channel_quality) + dst->num * sizeof(struct channel_quality_report);
				return 0;
			}
			case (WIFI_EXT_ASSOC_STATUS_REPLY):
			case (WIFI_EXT_ASSOC_TRIGGERRED):
			case (WIFI_EXT_DIASSOC_TRIGGERED):
			case (WIFI_EXT_DISASSOC_CONFIG): {
				struct wifi_assoc_status* src;
				struct ofl_ext_wifi_msg_assoc* dst;
				if (*len < sizeof(struct wifi_assoc_status))
				{
					OFL_LOG_WARN(LOG_MODULE, "Received  message wifi_assoc_status has invalid length (%zu).", *len);
                    return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
				}
				OFL_LOG_WARN(LOG_MODULE, "unpack wifi_assoc_status");
				src = (struct wifi_assoc_status*)exp;
				dst = (struct ofl_ext_wifi_msg_assoc*)malloc(sizeof(struct ofl_ext_wifi_msg_assoc));
				dst->header.header.experimenter_id = ntohl(exp->vendor);
				OFL_LOG_DBG(LOG_MODULE, "dst->header.header.experimenter_id : %x", dst->header.header.experimenter_id );
				dst->header.type = ntohl(exp->subtype);
				dst->num = ntohl (src->num);
				dst->addresses = (struct sta_address**)malloc(dst->num * struct sta_address*);
				for (size_t i = 0; i < dst->num; ++i)
				{
					dst->addresses[i] = (struct sta_address*)malloc(struct sta_address);
					memcpy(dst->addresses[i]->mac48address, src->status[i]->mac48address);
				}
				(*msg) = (struct ofl_msg_experimenter*)dst;
				*len -= sizeof(struct wifi_assoc_status) + dst->num * sizeof(struct assoc_status);
				break;
			}
			case (WIFI_EXT_DISASSOC_CONFIG_REPLY):
			case (WIFI_EXT_ASSOC_CONFIG): {
				struct wifi_assoc_disassoc_config* src;
				struct ofl_ext_wifi_msg_assoc_disassoc_config* dst;
				if (*len < sizeof(struct wifi_assoc_disassoc_config))
				{
					OFL_LOG_WARN(LOG_MODULE, "Received  message wifi_assoc_disassoc_config has invalid length (%zu).", *len);
                    return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
				}
				OFL_LOG_WARN(LOG_MODULE, "unpack wifi_assoc_disassoc_config");
				src = (struct wifi_assoc_disassoc_config*)exp;
				dst = (struct ofl_ext_wifi_msg_assoc_disassoc_config*)malloc(sizeof(struct ofl_ext_wifi_msg_assoc_disassoc_config));
				dst->header.header.experimenter_id = ntohl(exp->vendor);
				OFL_LOG_DBG(LOG_MODULE, "dst->header.header.experimenter_id : %x", dst->header.header.experimenter_id );
				dst->header.type = ntohl(exp->subtype);
				dst->len = ntohl(src->len);
				memcpy(dst->mac48address, src->mac48address, 6);
				dst->data = (uint8_t*)malloc(dst->len);
				memcpy(dst->data, src->data, dst->len);
				(*msg) = (struct ofl_msg_experimenter*)dst;
				*len -= sizeof(struct wifi_assoc_disassoc_config) + dst->len;
				break;
			}
			default: {
				OFL_LOG_WARN(LOG_MODULE, "Trying to unpack unknown wifi Experimenter message.");
                return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_EXPERIMENTER);
			}
		}
	}else {
		OFL_LOG_WARN(LOG_MODULE, "Trying to unpack non-wifi Experimenter message.");
		return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_EXPERIMENTER);
	}
}

int
ofl_exp_wifi_msg_free(struct ofl_msg_experimenter *msg)
{
	if (msg->experimenter_id == WIFI_VENDOR_ID) {
		struct ofl_exp_wifi_msg_header *exp = (struct ofl_exp_wifi_msg_header*)msg;
		switch (exp->type) {
			case (WIFI_EXT_CHANNEL_CONFIG_REQUEST):
			case (WIFI_EXT_ASSOC_STATUS_REQUEST):
				break;
			case (WIFI_EXT_CHANNEL_CONFIG_REPLY):
			case (WIFI_EXT_CHANNEL_SET): {
				struct ofl_exp_wifi_msg_channel *c = (struct ofl_exp_wifi_msg_channel *)exp;
				free (c->channel);
				break;
			}
			case (WIFI_EXT_CHANNEL_QUALITY_REQUEST):
				break;
			case (WIFI_EXT_CHANNEL_QUALITY_REPLY):
			case (WIFI_EXT_CHANNEL_QUALITY_TRIGGER_SET):
			case (WIFI_EXT_CHANNEL_QUALITY_TRIGGERED):
			{
				struct ofl_exp_wifi_msg_chaqua *c = (struct ofl_exp_wifi_msg_chaqua *)exp;
				for (uint32_t i = 0; i < c->num; ++i)
				{
					free (c->reports[i]);
				}
				break;
			}
			case (WIFI_EXT_ASSOC_STATUS_REPLY):
			case (WIFI_EXT_ASSOC_TRIGGERRED):
			case (WIFI_EXT_DIASSOC_TRIGGERED):
			case (WIFI_EXT_DISASSOC_CONFIG):
			{
				struct ofl_ext_wifi_msg_assoc* c = (struct ofl_ext_wifi_msg_assoc*)exp;
				for (uint32_t i = 0; i < c->num; ++i)
				{
					free(c->addresses[i]);
				}
				free(c->addresses);
				break;
			}
			case (WIFI_EXT_DISASSOC_CONFIG_REPLY):
			case (WIFI_EXT_ASSOC_CONFIG):
			{
				struct ofl_ext_wifi_msg_assoc_disassoc_config* c = (struct ofl_ext_wifi_msg_assoc_disassoc_config*)exp;
				free(c->data);
				break;
			}
			default: {
				OFL_LOG_WARN(LOG_MODULE, "Trying to free unknown wifi Experimenter message.");
			}
				
		}
	}
	else {
		OFL_LOG_WARN(LOG_MODULE, "Trying to free non-wifi Experimenter message.");
	}
	free(msg);
	return 0;
}

void wifi_assoc_status_to_string (FILE* stream, struct ofl_msg_experimenter *msg)
{
	struct ofl_ext_wifi_msg_assoc *exp = (struct ofl_ext_wifi_msg_assoc *)msg;
	for (uint32_t i = 0; i < exp->num; ++i)
	{
		fprintf(stream, "sta=%s", exp->addresses[i]->mac48address);
	}
}

void wifi_assoc_disassoc_config_to_string(FILE* stream, struct ofl_msg_experimenter *msg)
{
	struct ofl_ext_wifi_msg_assoc_disassoc_config *exp = (struct ofl_ext_wifi_msg_assoc_disassoc_config*)msg;
	fprintf(stream, "sta=%s", exp->mac48address);
	fprintf(stream, "len=%u", exp->len);
	//TODO: print data
}

char *
ofl_exp_wifi_msg_to_string(struct ofl_msg_experimenter *msg)
{
	char *str;
    size_t str_size;
    FILE *stream = open_memstream(&str, &str_size);
	
	if (msg->experimenter_id == WIFI_VENDOR_ID) {
		struct ofl_exp_wifi_msg_header *exp = (struct ofl_exp_wifi_msg_header*)msg;
		switch (exp->type) {
			case (WIFI_EXT_CHANNEL_CONFIG_REQUEST):
				fprintf (stream, "request for initial channel configure");
				break;
			case (WIFI_EXT_CHANNEL_CONFIG_REPLY):
			case (WIFI_EXT_CHANNEL_SET): {
				struct ofl_exp_wifi_msg_channel *c = (struct ofl_exp_wifi_msg_channel *)exp;
				fprintf (stream, "%s channel configure, ", 
						 exp->type == WIFI_EXT_CHANNEL_CONFIG_REPLY ? "reply" : "set");
				fprintf (stream, "channel number = %u,", c->channel->m_channelNumber);
				fprintf (stream, "frequency = %u, ", c->channel->m_frequency);
				fprintf (stream, "chanel width = %u, ", c->channel->m_channelWidth);
				fprintf (stream, "mac48address = %s.", c->mac48address);
				break;
			}
			case (WIFI_EXT_CHANNEL_QUALITY_REQUEST):
			{
				struct ofl_exp_wifi_msg_chaqua_req *c = (struct ofl_exp_wifi_msg_chaqua_req*)exp;
				fprintf (stream, "request for channel quality of: %s", c->mac48address);
				break;
			}
			case (WIFI_EXT_CHANNEL_QUALITY_REPLY):
			case (WIFI_EXT_CHANNEL_QUALITY_TRIGGER_SET):
			case (WIFI_EXT_CHANNEL_QUALITY_TRIGGERED):
			{
				struct ofl_exp_wifi_msg_chaqua* c = (struct ofl_exp_wifi_msg_chaqua*)exp;
				if (exp->type == WIFI_EXT_CHANNEL_QUALITY_REPLY)
					fprintf (stream, "Type: WIFI_EXT_CHANNEL_QUALITY_REPLY");
				else if (exp->type == WIFI_EXT_CHANNEL_QUALITY_TRIGGER_SET)
					fprintf (stream, "Type: WIFI_EXT_CHANNEL_QUALITY_TRIGGER_SET");
				else
					fprintf (stream, "Type: WIFI_EXT_CHANNEL_QUALITY_TRIGGERED");
				fprintf (stream, "num: %u", c->num);
				for (uint32_t i = 0; i < c->num; ++i)
				{
					fprintf (stream, "mac48address: %s", c->reports[i]->mac48address);
					fprintf (stream, "packets: %lu", c->reports[i]->packets);
					fprintf (stream, "rxPower_avg: %f", c->reports[i]->rxPower_avg);
					fprintf (stream, "rxPower_std: %f", c->reports[i]->rxPower_std);
				}
				break;
			}
			case (WIFI_EXT_ASSOC_STATUS_REQUEST): {
				fprintf (stream, "request for initial association status");
				break;
			}
			case (WIFI_EXT_ASSOC_STATUS_REPLY): {
				fprintf (stream, "reply for initial association status");
				wifi_assoc_status_to_string(stream, msg);
				break;
			}
			case (WIFI_EXT_ASSOC_TRIGGERRED): {
				fprintf (stream, "new associated STA trigger msg");
				wifi_assoc_status_to_string(stream, msg);
				break;
			}
			case (WIFI_EXT_DIASSOC_TRIGGERED): {
				fprintf (stream, "disassociated STA trigger msg");
				wifi_assoc_status_to_string(stream, msg);
				break;
			}
			case (WIFI_EXT_DISASSOC_CONFIG): {
				fprintf (stream, "disassocate STA from AP config msg");
				wifi_assoc_status_to_string(stream, msg);
				break;
			}
			case (WIFI_EXT_DISASSOC_CONFIG_REPLY): {
				fprintf (stream, "disassocate STA from AP config reply");
				wifi_assoc_disassoc_config_to_string(stream, msg);
				break;
			}
			case (WIFI_EXT_ASSOC_CONFIG): {
				fprintf (stream, "associate STA to AP config msg");
				wifi_assoc_disassoc_config_to_string(stream, msg);
				break;
			}
			default: {
				OFL_LOG_WARN(LOG_MODULE, "Trying to print unknown wifi Experimenter message.");
                fprintf(stream, "ofexp{type=\"%u\"}", exp->type);
			}
		}
	}else {
		OFL_LOG_WARN(LOG_MODULE, "Trying to print non-wifi Experimenter message.");
        fprintf(stream, "exp{exp_id=\"%u\"}", msg->experimenter_id);
	}
	fclose(stream);
    return str;
}
