/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2004,2005,2006 INRIA
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
 * Author: xyy <xyynku@163.com>
 */

#include "ns3/log.h"
#include "sdn-wifi-manager.h"
#include "wifi-tx-vector.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("SdnWifiManager");
NS_OBJECT_ENSURE_REGISTERED (SdnWifiManager);

TypeId
SdnWifiManager::GetTypeId(void)
{
	
}

SdnWifiManager::SdnWifiManager()
{
	
}

SdnWifiManager::~SdnWifiManager()
{
	
}

void
SdnWifiManager::DoReportRxOk (WifiRemoteStation *station,
								   double rxSnr, WifiMode txMode)
{
	
}

} // namespace ns3
