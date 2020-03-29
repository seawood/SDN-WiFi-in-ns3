/* Copyright (c) 2020, Peking University
 * Author: xie yingying <xyynku@163.com>
**/

#ifndef DP_EXP_WIFI_H
#define DP_EXP_WIFI_H 1

#include "datapath.h"
#include "oflib-exp/ofl-exp-wifi.h"
#include "oflib/ofl.h"

#if defined (__GNUC__) && defined (NS3_OFSWITCH13)
ofl_err dp_handle_wifi_config_request(struct datapath *dp, struct ofl_exp_wifi_msg_channel_req *msg,
									  const struct sender *sender);					  

ofl_err dp_handle_wifi_channel_set(struct datapath *dp, struct ofl_exp_wifi_msg_channel *msg,
									const struct sender *sender);					  
#endif

#endif // DP_EXP_WIFI_H
