/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef WIFI_NET_DEVICE_H
#define WIFI_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/packet.h"
#include "ns3/callback-trace-source.h"
#include "ns3/mac48-address.h"
#include "ssid.h"
#include <string>

namespace ns3 {

class WifiChannel;
class WifiPhy;
class MacStations;
class MacLow;
class MacRxMiddle;
class MacTxMiddle;
class MacParameters;
class DcaTxop;
class MacHighAdhoc;
class MacHighNqsta;
class MacHighNqap;

/**
 * \brief the base class for 802.11 network interfaces
 *
 */
class WifiNetDevice : public NetDevice {
public:
  virtual ~WifiNetDevice ();

  /**
   * \param channel the channel to connect this 802.11 
   *        interface to.
   */
  void ConnectTo (Ptr<WifiChannel> channel);

  /**
   * \returns the Mac48Address of this 802.11 interface.
   *
   * This method is equivalent to NetDevice::GetAddress. The only
   * difference is its return type.
   */
  Mac48Address GetSelfAddress (void) const;
  /**
   * \returns the bssid of this 802.11 interface.
   */
  virtual Mac48Address GetBssid (void) const = 0;
  /**
   * \returns the ssid of this 802.11 interface.
   */
  virtual Ssid GetSsid (void) const = 0;

private:
  // inherited from parent.
  virtual bool DoNeedsArp (void) const;
  virtual Ptr<Channel> DoGetChannel (void) const;
  virtual bool SendTo (const Packet &packet, const Address &to, uint16_t protocolNumber);
  // defined for children
  virtual void NotifyConnected (void) = 0;
  virtual bool DoSendTo (const Packet &packet, const Mac48Address &to) = 0;
  // private helper
  void Construct (void);

  friend class WifiNetDeviceFactory;

  CallbackTraceSource<Packet, Mac48Address> m_rxLogger;
  CallbackTraceSource<Packet, Mac48Address> m_txLogger;
protected:
  WifiNetDevice (Ptr<Node> node);
  void DoForwardUp (Packet packet, const Mac48Address &from);
  virtual void DoDispose (void);
  DcaTxop *CreateDca (uint32_t minCw, uint32_t maxCw) const;

  Ptr<WifiChannel> m_channel;
  WifiPhy *m_phy;
  MacStations *m_stations;
  MacLow *m_low;
  MacRxMiddle *m_rxMiddle;
  MacTxMiddle *m_txMiddle;
  MacParameters *m_parameters;
};

/**
 * \brief a 802.11 adhoc network interface
 *
 * This network interface is a very simple pass-through 
 * from the higher layers down to the MAC DCF layer.
 */
class AdhocWifiNetDevice : public WifiNetDevice {
public:
  AdhocWifiNetDevice (Ptr<Node> node);
  virtual ~AdhocWifiNetDevice ();

  virtual Mac48Address GetBssid (void) const;
  virtual Ssid GetSsid (void) const;
  void SetSsid (Ssid ssid);

protected:
  virtual void DoDispose (void);
private:
  void ForwardUp (void);
  virtual bool DoSendTo (const Packet &packet, Mac48Address const & to);
  virtual void NotifyConnected (void);

  Ssid m_ssid;
  DcaTxop *m_dca;
  MacHighAdhoc *m_high;
};

/**
 * \brief a 802.11 STA network interface 
 *
 * This network interface implements the MAC-level STA 
 * active probing, association, and disassociation prototols.
 *
 * By default, it starts a new probing phase whenever a new 
 * data packet must be sent and the STA is not yet associated
 * to the AP.
 */
class NqstaWifiNetDevice : public WifiNetDevice 
{
public:
  NqstaWifiNetDevice (Ptr<Node> node);
  virtual ~NqstaWifiNetDevice ();

  virtual Mac48Address GetBssid (void) const;
  virtual Ssid GetSsid (void) const;

  /**
   * \param ssid the ssid we want to associate with
   *
   * Start a new active probing phase with the specified
   * ssid.
   */
  void StartActiveAssociation (Ssid ssid);
protected:
  virtual void DoDispose (void);
private:
  void Associated (void);
  void DisAssociated (void);
  virtual bool DoSendTo (const Packet &packet, Mac48Address const & to);
  virtual void NotifyConnected (void);
  friend class WifiNetDeviceFactory;
  Ssid m_ssid;
  DcaTxop *m_dca;
  MacHighNqsta *m_high;
};

/**
 * \brief a 802.11 AP network interface
 *
 * This network interface implements the MAC-level
 * AP-side of the beacon, probing, and, association
 * protocols. By default, every STA which tries
 * to associate is accepted.
 */
class NqapWifiNetDevice : public WifiNetDevice 
{
public:
  NqapWifiNetDevice (Ptr<Node> node);
  virtual ~NqapWifiNetDevice ();

  virtual Mac48Address GetBssid (void) const;
  virtual Ssid GetSsid (void) const;
  void SetSsid (Ssid ssid);
protected:
  virtual void DoDispose (void);
private:
  virtual bool DoSendTo (const Packet &packet, Mac48Address const & to);
  virtual void NotifyConnected (void);
  friend class WifiNetDeviceFactory;
  Ssid m_ssid;
  DcaTxop *m_dca;
  MacHighNqap *m_high;
};

} // namespace ns3

#endif /* WIFI_NET_DEVICE_H */
