/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Authors: Alessandro Paganelli <alessandro.paganelli@unimore.it>
 *          Daniela Saladino <daniela.saladino@unimore.it>
 */

#ifndef MULTIMEDIAAPPLICATIONSENDER_H_
#define MULTIMEDIAAPPLICATIONSENDER_H_

/* FIXME: these includes are valid only for ns-3.10 or lower! */
#include <queue>
#include <cstdlib>
#include <cassert>
#include "ns3/inet-socket-address.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/data-rate.h"
#include "ns3/address.h"
#include "ns3/event-id.h"
#include "ns3/packetizer.h"
#include "ns3/rtp-protocol.h"
#include "ns3/nal-unit-header.h"
#include "ns3/fragmentation-unit-header.h"
#include "ns3/simulation-dataset.h"
#include "ns3/packet-trace-structure.h"

/* FIXME: this value will be used to provide enough space to
 * store any given packet created by the packetizer. It should be equal to the
 * allowed payload given by the network MTU */
#define _MAX_PACKET_LENGTH 100000

namespace ns3
{

  class MultimediaApplicationSender : public Application
  {
  protected:
    // FIXME: verify if the attributes are correct!
    Ptr<Node> m_node;
    SimulationDataset* m_simulationDataset; // FIXME: change from pointer to smart-pointer
    Packetizer* m_packetizer;
    Ipv4Address m_receiverIpAddress;
    int m_receiverPort;
    Address m_receiverSocketAddress;
    RtpProtocol* m_rtpProtocol; // FIXME: change from pointer to smart-pointer
    // FIXME: decide whether it is necessary or not!

    bool m_useTcp;
    Ptr<Socket> m_socket;
    bool m_running;
    bool m_lastPacket;
    uint8_t* m_packetBuffer;

    std::vector<SenderTraceRow> m_senderTrace;

    // Queue used to temporarily store packets waiting for the transmission
    std::queue<Ptr<Packet> > m_packetQueue;

    void
    ScheduleTx(unsigned int payloadLength);
    void
    SendPacket(void);

  private:
    virtual void
    StartApplication(void);
    virtual void
    StopApplication(void);

  public:
    MultimediaApplicationSender(Ptr<Node> node, Packetizer* packetizer,
        SimulationDataset* simulationDataset);
    MultimediaApplicationSender(Ptr<Node> node, Packetizer* packetizer,
        SimulationDataset* simulationDataset, bool useTcp);

    virtual
    ~MultimediaApplicationSender()
    {
      if (m_packetBuffer)
        {
          free(m_packetBuffer);
        }
    }

    void
    Init(Ptr<Node> node, Packetizer* packetizer,
        SimulationDataset* simulationDataset, bool useTcp);
    void
    SetupDestinationAddress(Ipv4Address address, int destinationPort);
  };

} // namespace ns3

#endif /* MULTIMEDIAAPPLICATIONSENDER_H_ */
