/* Copyright (c) 2020, Peking University
 * Author: xie yingying <xyynku@163.com>
**/

#include "vlog.h"
#include "dp_exp_wifi.h"
#include "datapath.h"
#include "oflib-exp/ofl-exp-wifi.h"

#define UNUSED __attribute__((__unused__))

#define LOG_MODULE VLM_dp_exp_wifi

#if defined (__GNUC__) && defined (NS3_OFSWITCH13)
	// Define these functions as weak, 
	// so ns3 can override it and send the buffer over switch port.
	#pragma message "weak function: wifi"
	#pragma weak dp_handle_wifi_config_request
	#pragma weak dp_handle_wifi_channel_set
	#pragma weak dp_handle_wifi_chanqua_request
	#pragma weak dp_handle_wifi_chanqua_trigger_set
	#pragma weak dp_handle_wifi_assoc_status_request
    #pragma weak dp_handle_wifi_disassoc_config
    #pragma weak dp_handle_wifi_assoc_config
#endif

static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(60, 60);

ofl_err
dp_handle_wifi_config_request(struct datapath *dp UNUSED, struct ofl_exp_wifi_msg_channel_req *msg UNUSED,
							  const struct sender *sender UNUSED) {
	VLOG_DBG_RL(LOG_MODULE, &rl, "handle WIFI_EXT_CHANNEL_CONFIG_REQUEST msg");
	return 0;
}

ofl_err
dp_handle_wifi_channel_set(struct datapath *dp UNUSED, struct ofl_exp_wifi_msg_channel *msg UNUSED,
						   const struct sender *sender UNUSED) {
	VLOG_DBG_RL(LOG_MODULE, &rl, "handle WIFI_EXT_CHANNEL_SET msg");
	return 0;
}

ofl_err
dp_handle_wifi_chanqua_request(struct datapath *dp UNUSED,
							   struct ofl_exp_wifi_msg_chaqua_req *msg UNUSED,
							   const struct sender *sender UNUSED) {
	VLOG_DBG_RL(LOG_MODULE, &rl, "handle WIFI_EXT_CHANNEL_QUALITY_REQUEST msg");
	return 0;
}

ofl_err
dp_handle_wifi_chanqua_trigger_set (struct datapath *dp UNUSED,
									struct ofl_exp_wifi_msg_chaqua *msg UNUSED,
									const struct sender *sender UNUSED) {
	VLOG_DBG_RL(LOG_MODULE, &rl, "handle WIFI_EXT_CHANNEL_QUALITY_TRIGGER_SET msg");
	return 0;
}

ofl_err
dp_handle_wifi_assoc_status_request (struct datapath *dp, 
		struct ofl_exp_wifi_msg_channel_re *msg, 
		const struct sender *sender) {
	VLOG_DBG_RL(LOG_MODULE, &rl, "handle WIFI_EXT_ASSOC_STATUS_REQUEST msg");
	return 0;
}

ofl_err 
dp_handle_wifi_disassoc_config (struct datapath *dp, 
								struct ofl_ext_wifi_msg_assoc *msg, 
								const struct sender *sender) {
	VLOG_DBG_RL(LOG_MODULE, &rl, "handle WIFI_EXT_DISASSOC_CONFIG msg");
	return 0;
}

ofl_err 
dp_handle_wifi_assoc_config (struct datapath *dp, 
							struct ofl_ext_wifi_msg_assoc_disassoc_config *msg, 
							const struct sender *sender) {
	VLOG_DBG_RL(LOG_MODULE, &rl, "handle WIFI_EXT_ASSOC_CONFIG msg");
	return 0;
}
