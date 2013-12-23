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

#ifndef PACKETIZER_H_
#define PACKETIZER_H_

#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
}
#endif

#include <queue>
#include <cstring>
#include <cmath>
#include "stdint.h"
#include "simulation-dataset.h"
#include "ns3/rtp-protocol.h"
#include "ns3/nal-unit-header.h"
#include "ns3/fragmentation-unit-header.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"

namespace ns3
{

  class Packetizer
  {
  protected:
    // TODO: class' attributes
    unsigned int m_mtu;
    SimulationDataset* m_simulationDataset;
    std::queue<Ptr<Packet> > m_fragmentsQueue;

    /* A float value storing the samplingInterval */
    float m_samplingInterval;

    void
    CreateFragments(AVPacket* frame);

  public:
    Packetizer(int mtu, SimulationDataset* simulationDataset); // FIXME: change from pointer to smart-pointer
    virtual
    ~Packetizer() {};

    virtual bool
    GetNextPacket(unsigned int* packetId, unsigned long int* timestamp,
        uint8_t* buffer, unsigned int* packetSize) = 0;

    virtual bool
    GetNextPacket(Ptr<Packet>& packet) = 0;

    virtual uint32_t
    GetPayloadLength() = 0;
  };

} // namespace ns3

#endif /* PACKETIZER_H_ */
