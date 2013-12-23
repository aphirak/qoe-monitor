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

#include "packetizer.h"

#define _PACKETIZER_DEBUG 0

namespace ns3
{

  Packetizer::Packetizer(int mtu, SimulationDataset* simulationDataset) :
                         m_mtu(mtu),
                         m_fragmentsQueue(),
                         m_samplingInterval(0)
  {
    m_simulationDataset = simulationDataset;
  }

  /* Implements a naive fragmentation method, which produces packets of MTU size if the
   * current size exceeds the network's MTU */
  void
  Packetizer::CreateFragments(AVPacket* frame)
  {
    unsigned int currentSize = frame->size;
    unsigned int startingPointer = 0;
    uint8_t tempBuffer[m_mtu];
    unsigned int packetId = 0;
    PacketTraceRow currentRow;

    currentRow.m_packetSize = m_mtu;
    currentRow.m_playbackTimestamp = frame->pts * m_samplingInterval;
    currentRow.m_decodingTimestamp = frame->dts * m_samplingInterval;
    currentRow.m_rtpTimestamp = frame->dts; // FIXME: check if it is the dts or pts

    /* Calculate the number of fragments */
    currentRow.m_numberOfFragments = ceil(((float) currentSize) / m_mtu);

    /* Starting packet id extraction */
    unsigned int currentPacketTraceSize =
        (unsigned int) m_simulationDataset->GetPacketTrace().size();

    if (currentPacketTraceSize == 0)
      {
        packetId = 0;
      }
    else
      {
        packetId = (m_simulationDataset->GetPacketTrace().back()).m_packetId + 1;
      }

    /* Actual fragmentation */
    do
      {
        memcpy(tempBuffer, &frame->data[startingPointer], m_mtu);

        currentRow.m_packetId = packetId;

        /* Push back the trace row */
        m_simulationDataset->PushBackPacketTraceRow(currentRow);

        /* Packet creation */
        Ptr<Packet> packet = Create<Packet> (tempBuffer, m_mtu);

        /* Headers */
        if (startingPointer == 0)
          {
            /* First fragment - I must set the START bit flag */
            FragmentationUnitHeader fragHeader(FragmentationUnitHeader::START, 0x00);
            packet->AddHeader(fragHeader);
          }
        else
          {
            /* Middle fragment - Neither START nor END bit must be set */
            FragmentationUnitHeader fragHeader(FragmentationUnitHeader::UNSPECIFIED, 0x00);
            packet->AddHeader(fragHeader);
          }

        /* Add the NAL unit header */
        NalUnitHeader nalHeader(0x00, NalUnitHeader::FU_A);
        packet->AddHeader(nalHeader);

        /* Add the RTP header */
        RtpProtocol header(RtpProtocol::UNSPECIFIED, currentRow.m_packetId,
                           currentRow.m_rtpTimestamp, 0);
        packet->AddHeader(header);

        /* Push back the fragment into the queue */
        m_fragmentsQueue.push(packet);

#if _PACKETIZER_DEBUG
        std::cout << "Packetizer FRAGMENT data - packetId: " << currentRow.m_packetId
                  << ", timestamp: " << currentRow.m_rtpTimestamp
                  << ", packet size: " << currentRow.m_packetSize << "\n";
#endif

        packetId++;
        currentSize -= m_mtu;
        startingPointer += m_mtu;
      }
    while (currentSize > m_mtu);

    /* Last fragment */
    memcpy(tempBuffer, &frame->data[startingPointer], currentSize);

    currentRow.m_packetSize = currentSize;
    currentRow.m_packetId = packetId;

    /* Push back the trace row */
    m_simulationDataset->PushBackPacketTraceRow(currentRow);

    /* Packet creation */
    Ptr<Packet> packet = Create<Packet> (tempBuffer, currentSize);

    /* Headers */
    FragmentationUnitHeader fragHeader(FragmentationUnitHeader::END, 0x00);
    packet->AddHeader(fragHeader);

    /* Add the NAL unit header */
    NalUnitHeader nalHeader(0x00, NalUnitHeader::FU_A);
    packet->AddHeader(nalHeader);

    /* Add the RTP header */
    RtpProtocol header(RtpProtocol::UNSPECIFIED, currentRow.m_packetId,
                       currentRow.m_rtpTimestamp, 0);
    packet->AddHeader(header);

    /* Push back the fragment into the queue */
    m_fragmentsQueue.push(packet);

#if _PACKETIZER_DEBUG
    std::cout << "Packetizer FRAGMENT data - packetId: " << currentRow.m_packetId
              << ", timestamp: " << currentRow.m_rtpTimestamp
              << ", packet size: " << currentRow.m_packetSize << "\n";
#endif
  }
} // namespace ns3
