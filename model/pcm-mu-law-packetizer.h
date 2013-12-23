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

#ifndef PCMMULAWPACKETIZER_H_
#define PCMMULAWPACKETIZER_H_

#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
}
#endif

#include <queue>
#include "wav-container.h"
#include "packetizer.h"
#include "packet-trace-structure.h"
#include "rtp-protocol.h"

#define _RTP_PCM_PAYLOAD_SIZE 160

namespace ns3
{

  class PcmMuLawPacketizer : public Packetizer
  {
  private:
    /* The wav file container used to read the input file */
    WavContainer m_wavContainer;

    /* FIXME: Are the following variables useful or not?? */

    /* A byte queue exploitable by the packetization process */
    std::queue<uint8_t> m_packetizationQueue;

    /* A timestamp queue associated with the previous one, to extract the proper timestamp
     * when the actual packetization happens. */
    std::queue<unsigned long int> m_timestampQueue;

    void
    FillOnePacket(unsigned int* packetId, unsigned long int* timestamp,
        uint8_t* buffer, unsigned int* packetSize);

  public:
    PcmMuLawPacketizer(int mtu, SimulationDataset* simulationDataset);

    virtual uint32_t
    GetPayloadLength()
    {
      return _RTP_PCM_PAYLOAD_SIZE;
    }

    virtual bool
    GetNextPacket(unsigned int* packetId, unsigned long int* timestamp,
        uint8_t* buffer, unsigned int* packetSize);

    virtual bool
    GetNextPacket(Ptr<Packet>& packet);
  };

}

#endif /* PCMMULAWPACKETIZER_H_ */
