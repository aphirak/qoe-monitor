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

#ifndef MULTIMEDIAFILEREBUILDER_H_
#define MULTIMEDIAFILEREBUILDER_H_

#include <iostream>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string.h>
#include "ns3/packet.h"
#include "ns3/simulation-dataset.h"
#include "ns3/rtp-protocol.h"
#include "ns3/container.h"
#include "ns3/fragmentation-unit-header.h"

#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
}
#endif

namespace ns3
{

  class MultimediaFileRebuilder
  {
  protected:
    Container* m_outputContainer;
    RtpProtocol m_lastRtpHeader;
    bool m_startedReception;
    SimulationDataset* m_simulationDataset;

    std::string m_outputFormatName;

    /* A byte queue exploitable by the re-packetization process */
    std::queue<uint8_t> m_packetizationQueue;

    /* A timestamp queue associated with the previous one, to extract the proper timestamp
     * when the actual re-packetization happens. */
    std::queue<unsigned long int> m_timestampQueue;

    AVFormatContext m_codedFileFormatContext;
    AVFormatContext* m_outputFormatContext;
    AVOutputFormat* m_outputFormat;

    /* Packet queue used to temporarily store fragments */
    std::queue<Ptr<Packet> > m_fragmentsQueue;

    /* Stores the the first received fragment's rtp header */
    RtpProtocol m_lastFragmentRtpHeader;

    /* Packet buffer used to store the payload of a received packet */
    uint8_t* m_packetBuffer;

    /* Flag used to determine if the first fragment in the queue is actually a START fragment */
    bool m_isFirstStart;

    /* Method used to create a proxy packet based on the index passed as parameter. If packetId
     * refers to a fragment, the method automatically reconstructs the whole packet the fragment refers
     * to.
     * Returns the index of the next packet in the packet trace. */
    unsigned int
    CreateAndPushProxyPacket(unsigned int packetId);

    /* Method used to determine the packetId of the first fragment in a fragment burst */
    unsigned int
    GetStartingFragmentId(unsigned int fragmentId);

    void
    CheckLostFragments(RtpProtocol rtpHeader);

  public:
    // MultimediaFileRebuilder (SimulationDataset* dataset, std::string outputFormatName);
    MultimediaFileRebuilder(Container* outputContainer,
                            SimulationDataset* dataset);

    // FIXME: these methods have to be uniformed in their signatures
    void
    SetNextFragment(RtpProtocol rtpHeader, FragmentationUnitHeader fragHeader,
                    Ptr<Packet> fragment);

    void
    SetNextPacket(RtpProtocol rtpHeader, uint8_t* buffer, unsigned int packetSize);

    void
    FinalizeFile();
  };

} // namespace ns3

#endif /* MULTIMEDIAFILEREBUILDER_H_ */
