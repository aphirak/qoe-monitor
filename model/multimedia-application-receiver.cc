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

#include "multimedia-application-receiver.h"

namespace ns3
{

#define _MULTIMEDIA_APPLICATION_RECEIVER_DEBUG 0
#define _MULTIMEDIA_APPLICATION_RECEIVER_JITTER_DEBUG 0

  MultimediaApplicationReceiver::MultimediaApplicationReceiver(Ptr<Node> node,
      SimulationDataset* simulationDataset, Time jitterBufferLength) :
    m_socket(0)
  {
    Init(node, simulationDataset, jitterBufferLength, false);
  }

  MultimediaApplicationReceiver::MultimediaApplicationReceiver(Ptr<Node> node,
      SimulationDataset* simulationDataset, Time jitterBufferLength,
      bool useTcp) :
    m_socket(0)
  {
    Init(node, simulationDataset, jitterBufferLength, useTcp);
  }

  void
  MultimediaApplicationReceiver::Init(Ptr<Node> node,
      SimulationDataset* simulationDataset, Time jitterBufferLength,
      bool useTcp)
  {
    m_node = node;
    m_simulationDataset = simulationDataset;
    m_useTcp = useTcp;
    m_jitterBufferLength = jitterBufferLength;
    m_startedReception = false;
    m_currentJitterEstimate = 0;
    m_fileType = m_simulationDataset->GetFileType();

    m_packetBuffer = (uint8_t*) calloc(_RECEIVER_PACKET_BUFFER_LENGTH, sizeof(uint8_t));
    assert(m_packetBuffer != NULL);

    /* Create the receiver's side socket */
    if (!m_useTcp)
      {
        m_socket = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
      }
    else
      {
        m_socket = Socket::CreateSocket(m_node, TcpSocketFactory::GetTypeId());
      }
  }

  void
  MultimediaApplicationReceiver::StartApplication(void)
  {
#if _MULTIMEDIA_APPLICATION_RECEIVER_DEBUG
    std::cout << "Multimedia application receiver started\n";
#endif

    m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_receiverPort));

    /* Now I have to setup the receiver's callback, which is used upon
     * reception of each packet. */

    // TODO: Master the MakeCallback concept because it is not so clear! Why do I have to add "this" ??
    m_socket->SetRecvCallback(MakeCallback(
        &MultimediaApplicationReceiver::OnReceive, this));
  }

  void
  MultimediaApplicationReceiver::OnReceive(Ptr<Socket> socket)
  {
#if _MULTIMEDIA_APPLICATION_RECEIVER_DEBUG
    std::cout << "OnReceived called\n";
#endif

    Ptr<Packet> packet;
    RtpProtocol rtpHeader;
    NalUnitHeader nalHeader;

    /* The receiver side business logic goes here */
    while (packet = socket->Recv())
      {
        packet->RemoveHeader(rtpHeader);

        /* I must decide whether the packet arrived in time or not */
        if (CheckJitter(rtpHeader))
          {
            m_lastReceivedTime = Simulator::Now().GetSeconds();

            /* Now I fill the receiver trace */
            ReceiverTraceRow receiverTraceRow;
            receiverTraceRow.m_packetId = rtpHeader.GetPacketId();
            receiverTraceRow.m_receiverTimestamp = m_lastReceivedTime;

            /* I push the current row to the stored trace */
            m_simulationDataset->PushBackReceiverTraceRow(receiverTraceRow);

            /* Now I must check if the current packet belongs to an AUDIO or a VIDEO file */
            if (m_fileType == SimulationDataset::AUDIO)
              {
                /* TODO - audio reception */

                /* I pass the current packet to the rebuilder */
                unsigned int packetSize = packet->CopyData(m_packetBuffer,
                    _RECEIVER_PACKET_BUFFER_LENGTH);
                m_fileRebuilder->SetNextPacket(rtpHeader, m_packetBuffer,
                    packetSize);
              }
            else if (m_fileType == SimulationDataset::VIDEO)
              {
                /* Video reception */

                /* Nal unit header extraction */
                packet->RemoveHeader(nalHeader);

                /* Check the Nal type */
                if (nalHeader.GetType() == nalHeader.NAL_UNIT)
                  {
                    /* Single Nal unit */
                    /* I pass the current packet to the rebuilder */
                    unsigned int packetSize = packet->CopyData(m_packetBuffer,
                        _RECEIVER_PACKET_BUFFER_LENGTH);
                    m_fileRebuilder->SetNextPacket(rtpHeader, m_packetBuffer,
                        packetSize);

#if _MULTIMEDIA_APPLICATION_RECEIVER_DEBUG
                    std::cout << "NAL UNIT received at: " << Simulator::Now().GetSeconds () << " seconds\n";
                    std::cout << "  receiver rtp header info: ";
                    rtpHeader.Print(std::cout);
#endif
                  }
                else if (nalHeader.GetType() == nalHeader.FU_A)
                  {
#if _MULTIMEDIA_APPLICATION_RECEIVER_DEBUG
                    std::cout << "FU.A fragment received at: " << Simulator::Now().GetSeconds () << " seconds\n";
                    std::cout << "  receiver rtp header info: ";
                    rtpHeader.Print(std::cout);
#endif
                    /* Fragmentation unit - remove header */
                    FragmentationUnitHeader fragHeader;
                    packet->RemoveHeader(fragHeader);

                    /* Now I pass the current fragment to the file rebuilder */
                    m_fileRebuilder->SetNextFragment(rtpHeader, fragHeader, packet);
                  }
              }
          }
#if _MULTIMEDIA_APPLICATION_RECEIVER_JITTER_DEBUG
        else
          {
            std::cout << "Packet dropped at: " << Simulator::Now().GetSeconds () << " seconds because of jitter excess\n";
          }
#endif
      }
  }

  void
  MultimediaApplicationReceiver::StopApplication(void)
  {
#if _MULTIMEDIA_APPLICATION_RECEIVER_DEBUG
    std::cout << "Multimedia application receiver stopped\n";
#endif

    if (m_socket)
      {
        m_socket->Close();
      }

    /* Finalize the output file */
    m_fileRebuilder->FinalizeFile();
  }

  /* This method returns true if the packet is within the jitter treshold
   * set by the user, false otherwise.  */
  bool
  MultimediaApplicationReceiver::CheckJitter(RtpProtocol rtpHeader)
  {
    if (!m_startedReception)
      {
        /* The packet must be accepted because it is the first one */
        m_startedReception = true;
        m_rtpLastHeader = rtpHeader;
        return true;
      }

    /* Otherwise I have to compare the measured packet interarrival time with the theoretical one. */
    Time now = Simulator::Now();

    /* The measured interarrival time is in seconds */
    double currentTime = now.GetSeconds();
    double measuredInterarrivalTime = currentTime - m_lastReceivedTime;

    double idealInterarrivalTime = (rtpHeader.GetPacketTimestamp()
        - m_rtpLastHeader.GetPacketTimestamp())
        * (m_simulationDataset->GetSamplingInterval());

#if _MULTIMEDIA_APPLICATION_RECEIVER_JITTER_DEBUG
    std::cout << "Ideal interarrival time: " << idealInterarrivalTime
    << ", measured interarrival time: " << measuredInterarrivalTime << "\n";
#endif

    /* Update the rtp header stored for the next comparison */
    m_rtpLastHeader = rtpHeader;

    /* Update the jitter estimate, according to RFC 3550 */
    double currentDifference = measuredInterarrivalTime - idealInterarrivalTime;
    m_currentJitterEstimate = m_currentJitterEstimate
        + (fabs(currentDifference) - m_currentJitterEstimate) / (16);

    /* Export the current jitter estimate to the simulation dataset */
    JitterTraceRow jitterTraceRow;
    jitterTraceRow.m_packetId = m_rtpLastHeader.GetPacketId();
    jitterTraceRow.m_receptionTime = currentTime;
    jitterTraceRow.m_jitter = m_currentJitterEstimate;
    m_simulationDataset->PushBackJitterTraceRow(jitterTraceRow);

#if _MULTIMEDIA_APPLICATION_RECEIVER_JITTER_DEBUG
    std::cout << "Current jitter estimate: " << std::fixed << std::setw(8)
    << std::setprecision(6) << m_currentJitterEstimate << "\n";
#endif

    /* Check the current jitter value, according to RFC 3550 */
    if (currentDifference <= m_jitterBufferLength.GetSeconds())
      {
        return true;
      }
    else
      {
        return false;
      }
  }
} // namespace ns3
