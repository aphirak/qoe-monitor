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

#include "pcm-mu-law-packetizer.h"
#include "packetizer.h"

#define _PCM_MU_LAW_DEBUG 0

namespace ns3
{

#define _PCM_MU_LAW_PACKETIZER_DEBUG 0

  PcmMuLawPacketizer::PcmMuLawPacketizer(int mtu,
      SimulationDataset* simulationDataset) :
    Packetizer(mtu, simulationDataset), m_wavContainer(
        simulationDataset->GetOriginalCodedFile(), WavContainer::READ, AVMEDIA_TYPE_AUDIO)
  {
    m_wavContainer.InitForRead();
    m_samplingInterval = m_wavContainer.GetSamplingInterval();
  }

  bool
  PcmMuLawPacketizer::GetNextPacket(Ptr<Packet>& packet)
  {
    unsigned int packetId = 0;
    unsigned long int timestamp = 0;
    uint8_t buffer[_RTP_PCM_PAYLOAD_SIZE];
    unsigned int packetSize;

    if (GetNextPacket(&packetId, &timestamp, buffer, &packetSize))
      {
        /* Initialize the packet */
        packet = Create<Packet> (buffer, packetSize);

        /* I create the RTP header
         * FIXME: currently I fix the synch. source always to 0 */
        RtpProtocol rtpHeader(RtpProtocol::UNSPECIFIED, packetId,
                              timestamp, 0);
        packet->AddHeader(rtpHeader);

        return true;
      }

    return false;
  }

  /* This method returns the next packet from the packet queue into the buffer. Moreover, it
   * fills the timestamp and the packetId, too.
   * Returns true if everything went well, false otherwise. */
  bool
  PcmMuLawPacketizer::GetNextPacket(unsigned int* packetId,
      unsigned long int* timestamp, uint8_t* buffer, unsigned int* packetSize)
  {
    /* Check the current size of the queue */
    if (m_packetizationQueue.size() >= _RTP_PCM_PAYLOAD_SIZE)
      {
        /* There are enough bytes stored in the queue to create an output packet */
        FillOnePacket(packetId, timestamp, buffer, packetSize);
      }
    else
      {
        /* A new frame has to be read from the file */
        AVPacket readFrame;
        if (!m_wavContainer.GetNextPacket(&readFrame))
          {
            /* EOF has been reached */
            return false;
          }

        /* EOF has NOT been reached */
        unsigned long int initialTimestamp = (unsigned long int) readFrame.pts;

        /* I fill the queue with the entire packet */
        for (int i = 0; i < readFrame.size; i++)
          {
            m_packetizationQueue.push(readFrame.data[i]);
            m_timestampQueue.push(initialTimestamp + i);
          }

        /* Now I dequeue exactly ONE packet */
        FillOnePacket(packetId, timestamp, buffer, packetSize);

        av_free_packet(&readFrame);
      }

    return true;
  }

  /* Method used to fill exactly one packet */
  void
  PcmMuLawPacketizer::FillOnePacket(unsigned int* packetId,
      unsigned long int* timestamp, uint8_t* buffer, unsigned int* packetSize)
  {
    /* The current value of the timestamp is the one that has to be put in the RTP header */
    PacketTraceRow currentRow;

    currentRow.m_packetSize = _RTP_PCM_PAYLOAD_SIZE;
    *packetSize = _RTP_PCM_PAYLOAD_SIZE;

    /* PCM packets do not require nor support fragmentation */
    currentRow.m_numberOfFragments = 1;

    /* NB: The timestamp that has to be exported is a floating point value obtained from
     * the integer value extracted from the format! Moreover, the decoding timestamp is set
     * equal to the presentation timestamp. */
    currentRow.m_rtpTimestamp = m_timestampQueue.front();
    currentRow.m_playbackTimestamp = (m_timestampQueue.front())
        * m_samplingInterval;
    currentRow.m_decodingTimestamp = currentRow.m_playbackTimestamp;

    /* I fill the output timestamp */
    *timestamp = m_timestampQueue.front();

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

    m_simulationDataset->PushBackPacketTraceRow(currentRow);

    /* Now I extract exactly _RTP_PCM_PAYLOAD_SIZE bytes from the queue */
    for (unsigned int i = 0; i < _RTP_PCM_PAYLOAD_SIZE; i++)
      {
        buffer[i] = m_packetizationQueue.front();
        m_packetizationQueue.pop();
        m_timestampQueue.pop();
      }

#if _PCM_MU_LAW_PACKETIZER_DEBUG
    std::cout << "Pcm mu-law packetizer data - packetId: " << *packetId << ", timestamp: " << *timestamp
    << ", packet size: " << *packetSize << "\n";
#endif
  }

}
