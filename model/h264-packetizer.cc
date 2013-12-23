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

#include <iostream>
#include <fstream>
#include <cassert>

#include "h264-packetizer.h"
#include "packetizer.h"

#define _H264_DEBUG 0
#define _IP_HEADER_LENGTH 20
#define _UDP_HEADER_LENGTH 8
#define _RTP_HEADER_LENGTH 12

namespace ns3
{

#define _H264_PACKETIZER_DEBUG 1

  H264Packetizer::H264Packetizer(int mtu,
                   SimulationDataset* simulationDataset) :
                   Packetizer(mtu, simulationDataset), 
                   m_mpeg4Container(simulationDataset->GetOriginalCodedFile(), 
                                    Container::READ, AVMEDIA_TYPE_VIDEO),
                   m_packetizationQueue(),
                   m_timestampQueue()
  {
    m_mpeg4Container.InitForRead();
    m_samplingInterval = m_mpeg4Container.GetSamplingInterval();
  }

  bool
  H264Packetizer::GetNextPacket(Ptr<Packet>& packet)
  {
    if (m_fragmentsQueue.size() > 0)
      {
        /* Fragments are already here, return one of them */
        packet = m_fragmentsQueue.front();
        m_fragmentsQueue.pop();

        return true;
      }
    else
      {
        /* No fragment(s) present. A packet must be read from the container */
        AVPacket readFrame;
        if (!m_mpeg4Container.GetNextPacket(&readFrame))
          {
            /* EOF has been reached */
            return false;
          }

        /* Check over the size of the packet */
        assert(readFrame.size > 0);
        unsigned int currentFrameSize = (unsigned int) readFrame.size;
        if (currentFrameSize + _IP_HEADER_LENGTH + _UDP_HEADER_LENGTH +
            _RTP_HEADER_LENGTH + 1 > m_mtu)
          {
            /* FRAGMENT packet */
            CreateFragments(&readFrame);

            /* Free the packet */
            av_free_packet(&readFrame);
            packet = m_fragmentsQueue.front();
            m_fragmentsQueue.pop();

            return true;
          }
        else
          {
            /* NO FRAGMENT packet */
            PacketTraceRow currentRow;

            currentRow.m_packetSize = readFrame.size;

            /* Timestamp extraction */
            currentRow.m_playbackTimestamp = readFrame.pts * m_samplingInterval;
            currentRow.m_decodingTimestamp = readFrame.dts * m_samplingInterval;
            currentRow.m_rtpTimestamp = readFrame.dts; // FIXME: check if it is the dts or pts
            currentRow.m_numberOfFragments = 1;

            /* Check the size of the packet trace: if it is zero, this means that no packet
             * has been traced yet. */
            unsigned int currentPacketTraceSize =
                (unsigned int) m_simulationDataset->GetPacketTrace().size();

            if (currentPacketTraceSize == 0)
              {
                currentRow.m_packetId = 0;
              }
            else
              {
                currentRow.m_packetId
                    = (m_simulationDataset->GetPacketTrace().back()).m_packetId + 1;
              }

            /* Push back the trace row */
            m_simulationDataset->PushBackPacketTraceRow(currentRow);

            /* Now I create the packet */
            packet = Create<Packet> (readFrame.data, readFrame.size);

            /* I create the NAL Unit Header */
            NalUnitHeader nalHeader(0x00, NalUnitHeader::NAL_UNIT);
            packet->AddHeader(nalHeader);

            /* I create the RTP header
             * FIXME: currently I fix the synch. source always to 0 */
            RtpProtocol rtpHeader(RtpProtocol::UNSPECIFIED, currentRow.m_packetId,
                                  currentRow.m_rtpTimestamp, 0);
            packet->AddHeader(rtpHeader);

#if _H264_PACKETIZER_DEBUG
            std::cout << "H264 packetizer data - packetId: " << currentRow.m_packetId
                      << ", timestamp: " << currentRow.m_rtpTimestamp
                      << ", packet size: " << currentRow.m_packetSize << "\n";
#endif

            /* Free the packet */
            av_free_packet(&readFrame);
            return true;
          }
      }
  }

  /* This method returns the next packet from the packet queue into buffer. Moreover, it
   * fills the timestamp and the packetId, too.
   * Returns true if everything went well, false otherwise. */
  bool
  H264Packetizer::GetNextPacket(unsigned int* packetId,
      unsigned long int* timestamp, uint8_t* buffer, unsigned int* packetSize)
  {
    /* A new frame has to be read from the file */
    AVPacket readFrame;
    if (!m_mpeg4Container.GetNextPacket(&readFrame))
      {
        /* EOF has been reached */
        return false;
      }

    /* Actual packetization */
    PacketTraceRow currentRow;
    
    /* Packet size extraction */
    currentRow.m_packetSize = readFrame.size;
    *packetSize = readFrame.size;

    /* Check if the current packet is smaller than MTU */
    assert((*packetSize + _IP_HEADER_LENGTH + _UDP_HEADER_LENGTH +
                          _RTP_HEADER_LENGTH) <= m_mtu);

    /* Timestamp extraction */
    currentRow.m_playbackTimestamp = readFrame.pts * m_samplingInterval;
    currentRow.m_decodingTimestamp = readFrame.dts * m_samplingInterval;
    currentRow.m_rtpTimestamp = readFrame.dts;

    /* Note that the receiver has to rebuild the packet and to present it to the user.
     * The timestamp has to be the presentation timestamp, otherwise the receiver could not
     * play the output stream! */
    *timestamp = readFrame.pts;

    /* Packet ID */
    
    /* Check the size of the packet trace: if it is zero, this means that no packet
     * has been traced yet. */
    unsigned int currentPacketTraceSize =
        (unsigned int) m_simulationDataset->GetPacketTrace().size();

    if (currentPacketTraceSize == 0)
      {
        currentRow.m_packetId = 0;
      }
    else
      {
        currentRow.m_packetId
            = (m_simulationDataset->GetPacketTrace().back()).m_packetId + 1;
      }

    /* I fill the output packetId */
    *packetId = currentRow.m_packetId;

    /* Push back the trace row */
    m_simulationDataset->PushBackPacketTraceRow(currentRow);

    /* Now I extract exactly readFrame.size bytes from the packet
     * FIXME: I could use memncpy
     * */

    assert(readFrame.size > 0);
    unsigned int currentFrameSize = (unsigned int) readFrame.size;
    for (unsigned int i = 0; i < readFrame.size; i++)
      {
        buffer[i] = readFrame.data[i];
      }

#if _H264_PACKETIZER_DEBUG
    std::cout << "H264 packetizer data - packetId: " << *packetId << ", timestamp: " << *timestamp
              << ", packet size: " << *packetSize << "\n";
#endif
    
    /* Free the packet */
    av_free_packet(&readFrame);

    return true;
  }
}
