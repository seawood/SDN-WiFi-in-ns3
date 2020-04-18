/* Copyright (c) 2020, Peking University
 * Author: xie yingying <xyynku@163.com>
**/

#ifndef DP_EXP_WIFI_H
#define DP_EXP_WIFI_H 1

#include "datapath.h"
#include "oflib-exp/ofl-exp-wifi.h"
#include "oflib/ofl.h"

ofl_err dp_handle_wifi_config_request(struct datapath *dp, struct ofl_exp_wifi_msg_channel_req *msg,
									  const struct sender *sender);					  

ofl_err dp_handle_wifi_channel_set(struct datapath *dp, struct ofl_exp_wifi_msg_channel *msg,
									const struct sender *sender);	

ofl_err dp_handle_wifi_chanqua_request(struct datapath *dp,
							   struct ofl_exp_wifi_msg_chaqua_req *msg,
							   const struct sender *sender);

ofl_err dp_handle_wifi_chanqua_trigger_set (struct datapath *dp,
									struct ofl_exp_wifi_msg_chaqua *msg,
									const struct sender *sender);

ofl_err dp_handle_wifi_assoc_status_request (struct datapath *dp, 
		struct ofl_exp_wifi_msg_channel_re *msg, 
		const struct sender *sender);

ofl_err dp_handle_wifi_disassoc_config (struct datapath *dp, 
										struct ofl_ext_wifi_msg_assoc *msg, 
										const struct sender *sender);

ofl_err dp_handle_wifi_assoc_config (struct datapath *dp, 
									 struct ofl_ext_wifi_msg_assoc_disassoc_config *msg, 
									 const struct sender *sender);
#endif // DP_EXP_WIFI_H
