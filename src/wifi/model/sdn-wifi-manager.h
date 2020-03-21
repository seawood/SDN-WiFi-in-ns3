
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

#ifndef SDN_WIFI_MANAGER_H
#define SDN_WIFI_MANAGER_H

#include "wifi-remote-station-manager.h"

namespace ns3{

class SdnWifimanager : public WifiRemoteStationManager
{
public:
	Static TypeId GetTypeId (void);
	SdnWifiManager ();
	virtual ~SdnWifiManager ();
	
	
	
private:
	//pure virtual method of WifiRemoteStationManager
	
	/**
   * MacLow::ReceiveOk()->SdnWifiManager::ReportRxOk()->DoReportRxOk()
   * record SNR
   * \param station the station that sent the DATA to us
   * \param rxSnr the SNR of the DATA we received
   * \param txMode the WifiMode the sender used to send the DATA
   */
	void DoReportRxOk (WifiRemoteStation *station,
                             double rxSnr, WifiMode txMode);
	/**
   * \param station the station that we failed to send RTS
   */
	void DoReportRtsFailed (WifiRemoteStation *station);
	  /**
	   * \param station the station that we failed to send DATA
	   */
	void DoReportDataFailed (WifiRemoteStation *station);
  /**
   * \param station the station that we successfully sent RTS
   * \param ctsSnr the SNR of the CTS we received
   * \param ctsMode the WifiMode the receiver used to send the CTS
   * \param rtsSnr the SNR of the RTS we sent
   */
  void DoReportRtsOk (WifiRemoteStation *station,
                              double ctsSnr, WifiMode ctsMode, double rtsSnr);
  /**
   * \param station the station that we successfully sent RTS
   * \param ackSnr the SNR of the ACK we received
   * \param ackMode the WifiMode the receiver used to send the ACK
   * \param dataSnr the SNR of the DATA we sent
   */
  void DoReportDataOk (WifiRemoteStation *station,
                               double ackSnr, WifiMode ackMode, double dataSnr);
  /**
   * \param station the station that we failed to send RTS
   */
  void DoReportFinalRtsFailed (WifiRemoteStation *station);
  /**
   * \param station the station that we failed to send DATA
   */
  void DoReportFinalDataFailed (WifiRemoteStation *station);
  
  /**
   * \return whether this manager is a manager designed to work in low-latency environments.
   *
   * Note: In this context, low vs high latency is defined in <i>IEEE 802.11 Rate Adaptation:
   * A Practical Approach</i>, by M. Lacage, M.H. Manshaei, and T. Turletti.
   */
  bool IsLowLatency (void) const;
  /**
   * \return a new station data structure
   */
  WifiRemoteStation* DoCreateStation (void) const;
  /**
    * \param station the station that we need to communicate
    *
    * \return the TXVECTOR to use to send a packet to the station
    *
    * Note: This method is called before sending a unicast packet or a fragment
    *       of a unicast packet to decide which transmission mode to use.
    */
  WifiTxVector DoGetDataTxVector (WifiRemoteStation *station);
  /**
   * \param station the station that we need to communicate
   *
   * \return the transmission mode to use to send an rts to the station
   *
   * Note: This method is called before sending an rts to a station
   *       to decide which transmission mode to use for the rts.
   */
  WifiTxVector DoGetRtsTxVector (WifiRemoteStation *station);
	
};
	
} // namespace ns3
#endif /* SDN_WIFI_MANAGER_H */
