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

#include "multimedia-application-sender.h"

namespace ns3
{

#define _MULTIMEDIA_APPLICATION_SENDER_DEBUG 1

/* Threshold used to determine if the packet has to be sent immediately or not, in case of packet-interleaving
 * or re-transmissions. */
#define _INTER_PACKET_TIME_THRESHOLD 0.5

#if _MULTIMEDIA_APPLICATION_SENDER_DEBUG
#define _DEBUG_NEGATIVE_TIMESTAMP 0
#define _DEBUG_OVER_THRESHOLD_TIMESTAMP 1
#endif

  MultimediaApplicationSender::MultimediaApplicationSender(Ptr<Node> node,
      Packetizer* packetizer, SimulationDataset* simulationDataset) :
    m_receiverSocketAddress(),
    m_socket(0),
    m_packetQueue(),
    m_lastPacket(false)
  {
    /* Default implementation requires UDP */
    Init(node, packetizer, simulationDataset, false);
  }

  MultimediaApplicationSender::MultimediaApplicationSender(Ptr<Node> node,
      Packetizer* packetizer, SimulationDataset* simulationDataset, bool useTcp) :
    m_receiverSocketAddress(),
    m_socket(0),
    m_lastPacket(false),
    m_packetQueue()
  {
    Init(node, packetizer, simulationDataset, useTcp);
  }

  void
  MultimediaApplicationSender::Init(Ptr<Node> node, Packetizer* packetizer,
      SimulationDataset* simulationDataset, bool useTcp)
  {
#if _MULTIMEDIA_APPLICATION_SENDER_DEBUG
    std::cout << "- MultimediaApplicationSender: initialization\n";
#endif

    m_simulationDataset = simulationDataset;
    m_useTcp = useTcp;
    m_node = node;
    m_packetizer = packetizer;
    m_rtpProtocol = NULL;
    m_running = false;

    /* Create the sender's side socket */
    if (!m_useTcp)
      {
        m_socket = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
      }
    else
      {
        m_socket = Socket::CreateSocket(m_node, TcpSocketFactory::GetTypeId());
      }

    /* Initialize the packet buffer */
    m_packetBuffer = (uint8_t*) calloc(_MAX_PACKET_LENGTH, sizeof(uint8_t));
    assert(m_packetBuffer != NULL);
  }

  void
  MultimediaApplicationSender::SetupDestinationAddress(Ipv4Address address,
      int destinationPort)
  {
#if _MULTIMEDIA_APPLICATION_SENDER_DEBUG
    std::cout << "- MultimediaApplicationSender: setup address\n";
#endif

    m_receiverIpAddress = address;
    m_receiverPort = destinationPort;
    m_receiverSocketAddress = InetSocketAddress(Ipv4Address(address),
        destinationPort);
  }

  void
  MultimediaApplicationSender::StartApplication(void)
  {
#if _MULTIMEDIA_APPLICATION_SENDER_DEBUG
    std::cout << "- MultimediaApplicationSender: started\n";
#endif

    m_running = true;
    m_socket->Bind();
    m_socket->Connect(m_receiverSocketAddress);
    SendPacket();
  }

  void
  MultimediaApplicationSender::StopApplication(void)
  {
#if _MULTIMEDIA_APPLICATION_SENDER_DEBUG
    std::cout << "- MultimediaApplicationSender: stopped\n";
#endif

    m_running = false;

    if (m_socket)
      {
        m_socket->Close();
      }
  }

  /* This is the core method of the class, responsible for creating and
   * sending data packets toward the receiver node.  */
  void
  MultimediaApplicationSender::SendPacket(void)
  {
    if (m_running)
      {
        /* First, I have to check the current packet queue length. There must be
         * at least two packets to properly schedule the transmissions. */
        while ((m_packetQueue.size() < 2) && m_running)
          {
            //Unused variable warning: packetId, timestamp, packetSize
            //unsigned int packetId = 0;
            //unsigned long int timestamp = 0;
            //unsigned int packetSize = 0;
            Ptr<Packet> packet;

            /* I ask to the packetizer for a new packet */
            // bool packetizerResult = m_packetizer->GetNextPacket(&packetId, &timestamp, m_packetBuffer, &packetSize);
            bool packetizerResult = m_packetizer->GetNextPacket(packet);

            if (!packetizerResult)
              {
                /* Stop the application, because we reached EOF, and send the last queued packet */
#if _MULTIMEDIA_APPLICATION_SENDER_DEBUG
                std::cout << "- MultimediaApplicationSender: EOF reached, I send the last queued packet.\n";
#endif
                /* Check the queue length - it should contain exactly one packet */
                assert(m_packetQueue.size() == 1);

                m_lastPacket = true;
                m_running = false;
              }
            else
              {
                /* Now the packet has to be enqueued into the packet queue */
                m_packetQueue.push(packet);
              }
          }

        /* The queue contains one or two packets. If it contains two packets, I can determine the actual
         * inter-packet interval by comparing the timestamps of the two packets. More specifically,
         * by subtracting the first timestamp to the second, together with sampling interval,
         * it is possible to determine when to schedule the second packet
         * stored in the queue. */

        /* I extract the first packet */
        Ptr<Packet> firstPacket = m_packetQueue.front();
        m_packetQueue.pop();

        /* I extract the first header */
        RtpProtocol firstHeader;
        firstPacket->PeekHeader(firstHeader);

        if (!m_lastPacket)
          {
            /* I obtain the two headers WITHOUT removing them */
            RtpProtocol secondHeader;
            m_packetQueue.front()->PeekHeader(secondHeader);

            unsigned long int firstTimestamp, secondTimestamp;
            firstTimestamp = firstHeader.GetPacketTimestamp();
            secondTimestamp = secondHeader.GetPacketTimestamp();

            // FIXME TODO: check possible overflows!
            long int timestampInterval = secondTimestamp - firstTimestamp;

            // FIXME TODO: verify if the procedure is correct by comparing the new traces
            // with the old ones!
            double nextPacketTime = m_simulationDataset->GetSamplingInterval() * timestampInterval;

            /* Now I have to check the calculated inter-packet interval:
             * - in case it is negative, packet interleaving could be employed. Our approach is to immediately
             *   send the considered packet back-to-back to the previous one.
             * - in case it is higher than _INTER_PACKET_TIME_THRESHOLD, the packet will be sent immediately as well.
             * FIXME TODO: It could be better to avoid sending a packet immediately. We could re-use the previous
             * inter-packet time... */
            if (nextPacketTime < 0.0)
              {
#if _DEBUG_NEGATIVE_TIMESTAMP
                std::cout << "- - MultimediaApplicationSender: Packet with NEGATIVE TIMESTAMP interval ("
                                          << timestampInterval << ", " << nextPacketTime << ")" << std::endl;
#endif
                nextPacketTime = 0.0;
              }
            else if (nextPacketTime > _INTER_PACKET_TIME_THRESHOLD)
              {
#if _DEBUG_OVER_THRESHOLD_TIMESTAMP
                std::cout << "- - MultimediaApplicationSender: Packet with OVER-THRESHOLD TIMESTAMP interval ("
                          << timestampInterval << ", " << nextPacketTime << ")" << std::endl;
#endif
                nextPacketTime = 0.0;
              }

            Time tNext(Seconds(nextPacketTime));
            Simulator::Schedule(tNext, &MultimediaApplicationSender::SendPacket, this);
          }

        /* Finally, I send the extracted packet over the socket */
        m_socket->Send(firstPacket);

#if _MULTIMEDIA_APPLICATION_SENDER_DEBUG
        std::cout << "- MultimediaApplicationSender: Packet sent at: " << Simulator::Now().GetSeconds() << " seconds \n";
        std::cout << "- MultimediaApplicationSender: Sender rtp header info:";
        firstHeader.Print(std::cout);
#endif

        /* Update of the sender trace */
        SenderTraceRow senderTraceRow;
        senderTraceRow.m_packetId = firstHeader.GetPacketId();
        senderTraceRow.m_senderTimestamp = Simulator::Now().GetSeconds();
        m_simulationDataset->PushBackSenderTraceRow(senderTraceRow);
      }
  }
} // namespace ns3
