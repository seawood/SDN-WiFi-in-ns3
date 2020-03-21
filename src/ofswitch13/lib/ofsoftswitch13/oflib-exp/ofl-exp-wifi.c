/* Copyright (c) 2020, Peking University
 * Author: xie yingying <xyynku@163.com>
**/

#define LOG_MODULE ofl_exp_wifi
OFL_LOG_INIT(LOG_MODULE)

int
ofl_exp_wifi_msg_pack(struct ofl_msg_experimenter *msg, 
					  uint8_t **buf, size_t *buf_len)
{
	if (msg->experimenter_id == WIFI_VENDOR_ID) {
		struct ofl_exp_wifi_msg_header *exp = (struct ofl_exp_wifi_msg_header*)msg;
		switch (exp->type){
			case (WIFI_EXT_CHANNEL_CONFIG_REQUEST): {
				struct ofl_exp_wifi_msg_channel_req* c = (struct ofl_exp_wifi_msg_channel_req*)exp;
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
		
}

ofl_err
ofl_exp_wifi_msg_unpack(struct ofp_header *oh, size_t *len,
						struct ofl_msg_experimenter **msg)
{
}

int
ofl_exp_wifi_msg_free(struct ofl_msg_experimenter *msg)
{
	if (msg->experimenter_id == WIFI_VENDOR_ID) {
		struct ofl_exp_wifi_msg_header *exp = (struct ofl_exp_wifi_msg_header*)msg;
		switch (exp->type) {
			case (WIFI_EXT_CHANNEL_CONFIG_REQUEST):
				break;
			case (WIFI_EXT_CHANNEL_CONFIG_REPLY):
			case (WIFI_EXT_CHANNEL_SET): {
				struct ofl_exp_wifi_msg_channel *c = (struct ofl_exp_wifi_msg_channel *)exp;
				free (c->channel);
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
				frpintf (stream, "frequency = %u, ", c->channel->m_frequency);
				fprintf (stream, "chanel width = %u.", c->channel->m_channelWidth);
				break;
			}
			default: {
				OFL_LOG_WARN(LOG_MODULE, "Trying to print unknown wifi Experimenter message.");
                fprintf(stream, "ofexp{type=\"%u\"}", exp->type);
			}
	}else {
		OFL_LOG_WARN(LOG_MODULE, "Trying to print non-wifi Experimenter message.");
        fprintf(stream, "exp{exp_id=\"%u\"}", msg->experimenter_id);
	}
	fclose(stream);
    return str;
}
