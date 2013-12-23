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

#include "multimedia-file-rebuilder.h"

namespace ns3
{

#define _MULTIMEDIA_FILE_REBUILDER_DEBUG 1

#define _MAX_PAYLOAD_LENGTH 2000
#define _PACKET_BUFFER_LENGTH 65536

  MultimediaFileRebuilder::MultimediaFileRebuilder(Container* outputContainer,
                                                   SimulationDataset* dataset) : 
    m_lastRtpHeader(RtpProtocol::UNSPECIFIED, -1, 0, 0)
  {
    m_outputContainer = outputContainer;
    m_simulationDataset = dataset;

    m_packetBuffer = (uint8_t*) calloc(_PACKET_BUFFER_LENGTH, sizeof(uint8_t));
    assert(m_packetBuffer != NULL);

    m_isFirstStart = false;
  }

  void
  MultimediaFileRebuilder::SetNextFragment(RtpProtocol rtpHeader, FragmentationUnitHeader fragHeader,
                                         Ptr<Packet> fragment)
  {
    if (!m_startedReception)
      {
        /* This is the first fragment of the reception. I save its Rtp header
         * FIXME: possible issues if I lose the first packets of the flow!! */
        m_lastFragmentRtpHeader = rtpHeader;
        m_startedReception = true;

        m_fragmentsQueue.push(fragment);

        if (fragHeader.IsStart())
          {
            m_isFirstStart = true;
          }
        else
          {
            m_isFirstStart = false;
          }
      }
    else
      {
        /* Here I must put the fragments management logic */

        /* Check if it is a START fragment */
        if (fragHeader.IsStart())
          {
#if _MULTIMEDIA_FILE_REBUILDER_DEBUG
            std::cout << " - MultimediaApplicationReceiver: START fragment received\n";
#endif

            /* Check the current status of the fragments queue. If it is not empty, a START
             * packet should not be here! It means that something went wrong. */
            if (m_fragmentsQueue.size() != 0)
              {
                /* This means that at least a fragment/packet has been lost.
                 * So I build a proxy packet for each possible packet lost. */
                unsigned int lostPacketId = m_lastFragmentRtpHeader.GetPacketId();

                /* Dequeue every previously stored fragment */
                while (m_fragmentsQueue.size() > 0)
                  {
                    m_fragmentsQueue.pop();
                  }

                /* Actual proxy packet building */
                unsigned int nextPacketId = 0;
                do
                  {
                    /* Build a proxy packet */
                    nextPacketId = CreateAndPushProxyPacket(lostPacketId);
                    lostPacketId = nextPacketId;
                  }
                while (nextPacketId < rtpHeader.GetPacketId());
              }

            /* Then i enqueue the START fragment */
            m_fragmentsQueue.push(fragment);
            m_lastFragmentRtpHeader = rtpHeader;
            m_isFirstStart = true;
          }

        /* Check if it is an END fragment */
        else if (fragHeader.IsEnd())
          {
#if _MULTIMEDIA_FILE_REBUILDER_DEBUG
            std::cout << "MultimediaApplicationReceiver: LAST fragment received\n";
#endif
            if ((m_fragmentsQueue.size() != 0) &&
                (m_lastFragmentRtpHeader.GetPacketTimestamp() == rtpHeader.GetPacketTimestamp()) &&
                (m_isFirstStart))
              {
                /* Last fragment. First I check if some fragments have been lost */
                CheckLostFragments(rtpHeader);

                m_fragmentsQueue.push(fragment);
                m_lastFragmentRtpHeader = rtpHeader;

                unsigned int queueLength = m_fragmentsQueue.size();

                /* Now I rebuild the equivalent received packet
                 * First I build a fake RTP header
                 * NB: this is a "hack" used to provide the container with all the information
                 * needed for its operations. In fact, it needs the packet ID of the LAST fragment
                 * (in order to check if there are lost packets) and the timestamp shared among
                 * all of them (to determine the fragment ownership to a given set of fragments). */
                RtpProtocol fakeHeader;

                fakeHeader.SetPacketId(m_lastFragmentRtpHeader.GetPacketId());
                fakeHeader.SetTimestamp(m_lastFragmentRtpHeader.GetPacketTimestamp());
                fakeHeader.SetSynchronizationSource(m_lastFragmentRtpHeader.GetSynchronizationSource());

                /* Now I rebuild the fragmented packet's payload
                 * I take care of the current offset in the buffer */
                unsigned int currentOffset = 0;

                for (unsigned int i = 0; i < queueLength; i++)
                  {
                    Ptr<Packet> currentPacket = m_fragmentsQueue.front();
                    m_fragmentsQueue.pop();

                    unsigned int currentPacketSize = currentPacket->CopyData(&m_packetBuffer[currentOffset], _MAX_PAYLOAD_LENGTH);

                    /* Update current offset */
                    currentOffset += currentPacketSize;
                  }

                /* Now I should have the whole packet available in the buffer. The overall length is
                 * provided by currentOffset */
                m_outputContainer->SetNextPacket(fakeHeader, m_packetBuffer, currentOffset);
                m_lastRtpHeader = fakeHeader;
                m_isFirstStart = false;
              }
            else
              {
                /* Something went wrong.
                 * I missed all the previous fragments of the burst, since the current
                 * one is and END fragment. */
                unsigned int nextPacketId = m_lastRtpHeader.GetPacketId() + 1; // todo: check if this works properly!

                /* Dequeue every possible previously stored fragment */
                while (m_fragmentsQueue.size() > 0)
                  {
                    m_fragmentsQueue.pop();
                  }

                do
                  {
                    /* Build a proxy packet */
                    nextPacketId = CreateAndPushProxyPacket(nextPacketId);
                  }
                while (nextPacketId < rtpHeader.GetPacketId());
              }
          }
        else
          {
#if _MULTIMEDIA_FILE_REBUILDER_DEBUG
            std::cout << "MultimediaApplicationReceiver: MIDDLE fragment received\n";
#endif
            if ((m_fragmentsQueue.size() != 0) &&
                (m_lastFragmentRtpHeader.GetPacketTimestamp() == rtpHeader.GetPacketTimestamp()))
              {
                /* Generic fragment (neither start, nor end).
                 * First, I must check if some fragments have been lost */
                CheckLostFragments(rtpHeader);
              }

            m_fragmentsQueue.push(fragment);
            m_lastFragmentRtpHeader = rtpHeader;

            if (m_fragmentsQueue.size() == 0)
              {
                m_isFirstStart = false;
              }
          }
      }
  }

  void
  MultimediaFileRebuilder::SetNextPacket(RtpProtocol rtpHeader,
                                         uint8_t* buffer, unsigned int packetSize)
  {
    /* I must check if I lost
     * some packets since the last one I received.
     * First, I check the packet ID */
    unsigned int currentPacketId = rtpHeader.GetPacketId();

    /* Now I compare the current packet ID with the one I'm expecting to receive */
    unsigned int lastPacketId = m_lastRtpHeader.GetPacketId(); 

    if (!m_startedReception)
      {
        m_startedReception = true;
      }
       
    if (currentPacketId != lastPacketId + 1)
      {
        /* At least one packet has been lost. Now I have to seek within
         * the packet trace to extract the exact size of each packet lost,
         * to properly fill the output buffer. */

#if _MULTIMEDIA_FILE_REBUILDER_DEBUG
        std::cout
          << "MultimediaFileRebuilder: packet lost detected! Last packet correctly received: "
          << lastPacketId << "\n";
#endif
        unsigned int nextPacketId = lastPacketId + 1;
        do
          {
            /* Build a proxy packet */
            nextPacketId = CreateAndPushProxyPacket(nextPacketId);
          }
        while (nextPacketId < rtpHeader.GetPacketId());
      }

    /* Now I can send the received packet to the output context */
    m_outputContainer->SetNextPacket(rtpHeader, buffer, packetSize);
    m_lastRtpHeader = rtpHeader;
  }

  /* FIXME TODO: verifica bene tutto!!! */
  unsigned int
  MultimediaFileRebuilder::CreateAndPushProxyPacket(unsigned int packetId)
  {
    /* First, I must check in the packet trace if the packetId refers to a fragment or
     * refers to a complete packet. */
    std::vector<PacketTraceRow> copyPacketTrace = m_simulationDataset->GetPacketTrace();
    PacketTraceRow currentPacketRow = copyPacketTrace[packetId];
    unsigned int currentNumberOfFragments = currentPacketRow.m_numberOfFragments;
    unsigned int startingId = GetStartingFragmentId(packetId);

    /* Now packetId equals the first actual packet of the bulk */
    unsigned int totalPacketLength = 0;
    unsigned int nextPacketId = startingId + currentNumberOfFragments;

    unsigned int i = startingId;
    do
      {
        totalPacketLength += copyPacketTrace[i].m_packetSize;
        i++;
      }
    while (i < nextPacketId);

#if 0
    for (unsigned int i = startingId; i < nextPacketId; i++)
      {
        totalPacketLength += copyPacketTrace[i].m_packetSize;
      }
#endif

    unsigned long int currentTimestamp = currentPacketRow.m_rtpTimestamp;

    /* Build the proxy packet */
    uint8_t* tempBuffer = (uint8_t*) calloc(totalPacketLength, sizeof(uint8_t));
    assert(tempBuffer != NULL);

    /* FIXME TODO check packetId! Has it to be the last or the first of the fragment burst? */
    RtpProtocol lostHeader = RtpProtocol(RtpProtocol::UNSPECIFIED,
                                         nextPacketId - 1, currentTimestamp, 0);

    m_outputContainer->SetNextPacket(lostHeader, tempBuffer, totalPacketLength);

    m_lastRtpHeader = lostHeader;

    free(tempBuffer);
    return nextPacketId;
  }

  unsigned int
  MultimediaFileRebuilder::GetStartingFragmentId(unsigned int fragmentId)
  {
    /* First, I must check in the packet trace if the packetId refers to a fragment or
     * refers to a complete packet. */
    std::vector<PacketTraceRow> copyPacketTrace = m_simulationDataset->GetPacketTrace();
    PacketTraceRow currentPacketRow = copyPacketTrace[fragmentId];
    unsigned int currentNumberOfFragments = currentPacketRow.m_numberOfFragments;
    unsigned int startingId = fragmentId;

    if (currentNumberOfFragments > 1 && fragmentId > 0)
      {
        /* Find the first fragment of the packet */
        int firstFragmentId = fragmentId - 1;

        while (copyPacketTrace[firstFragmentId].m_rtpTimestamp ==
               currentPacketRow.m_rtpTimestamp)
          {
            firstFragmentId--;
            if (firstFragmentId < 0)
              {
                break;
              }
          }

        /* The first actual fragment is the one found in the previous iteration */
        startingId = ++firstFragmentId;
      }

    return startingId;
  }

  void
  MultimediaFileRebuilder::CheckLostFragments(RtpProtocol rtpHeader)
  {
    unsigned int currentFragmentId = rtpHeader.GetPacketId();
    unsigned int lastFragmentId = m_lastFragmentRtpHeader.GetPacketId();

    if (currentFragmentId != (lastFragmentId + 1))
      {
        /* At least one fragment has been lost. Now I have to seek within
         * the packet trace to extract the exact size of each packet lost,
         * to properly fill the output buffer. */

#if _MULTIMEDIA_FILE_REBUILDER_DEBUG
        std::cout
            << "MultimediaFileRebuilder: fragment lost detected! Last fragment correctly received: "
            << lastFragmentId << "\n";
#endif

        for (unsigned int i = lastFragmentId + 1; i < currentFragmentId; i++)
          {
            unsigned int lostFragmentSize =
                m_simulationDataset->GetPacketTrace()[i].m_packetSize;
            unsigned long int lostRtpTimestamp =
                m_simulationDataset->GetPacketTrace()[i].m_rtpTimestamp;

            uint8_t* emptyBuffer = (uint8_t*) calloc(lostFragmentSize, sizeof(uint8_t));
            assert(emptyBuffer != NULL);

            /* Initialization to 0 */
            memset(emptyBuffer, 0x00, lostFragmentSize);

            RtpProtocol lostHeader = RtpProtocol(RtpProtocol::UNSPECIFIED,
                i, lostRtpTimestamp, 0);

            /* I create the "fake" packet */
            Ptr<Packet> fakeFragment = Create<Packet>(emptyBuffer, lostFragmentSize);

            /* Now I "set" this new fake packet to the fragment queue */
            m_fragmentsQueue.push(fakeFragment);
            m_lastFragmentRtpHeader = lostHeader;

            free(emptyBuffer);
          }
      }
  }

  void
  MultimediaFileRebuilder::FinalizeFile()
  {
    m_outputContainer->FinalizeFile();
    free(m_packetBuffer);
  }

} // namespace ns3
