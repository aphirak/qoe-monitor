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

#ifndef RTPPROTOCOL_H_
#define RTPPROTOCOL_H_

#include "ns3/header.h"

#define _FIRST_TWO_BYTES 0x8000 /* Definition of the first two bytes of the header */

namespace ns3
{

  class RtpProtocol : public Header
  {
  public:
    /* Declaration of the possible kinds of RTP payload.
     * FIXME: currently only one type is possible, namely, PCM. */
    enum RtpPayloadType
    {
      UNSPECIFIED, PCM
    };

    /* Constructor used at the receiver side */
    RtpProtocol()
    {
      m_payloadType = UNSPECIFIED;
      m_packetId = 0;
      m_timestamp = 0;
      m_firstTwoBytes = 0;
      m_synchronizationSource = 0;
    }

    /* Constructor used at the transmitter side */
    RtpProtocol(RtpPayloadType type, unsigned int packetId,
        unsigned long int timestamp, unsigned long int synchronizationSource)
    {
      m_payloadType = type;
      m_packetId = packetId;
      m_timestamp = timestamp;
      m_synchronizationSource = synchronizationSource;

      /* FIXME: this holds true only for PCM, which has PT = 0x00 */
      if (m_payloadType == PCM)
        {
          m_firstTwoBytes = _FIRST_TWO_BYTES;
        }
    }

    static TypeId
    GetTypeId(void);
    virtual TypeId
    GetInstanceTypeId(void) const;

    virtual void
    Print(std::ostream &os) const;
    virtual void
    Serialize(Buffer::Iterator start) const;
    virtual uint32_t
    Deserialize(Buffer::Iterator start);
    virtual uint32_t
    GetSerializedSize() const;

    // FIXME: decide whether to use references for return values
    unsigned long int
    GetPacketTimestamp()
    {
      return m_timestamp;
    }

    unsigned int
    GetPacketId()
    {
      return m_packetId;
    }

    unsigned long int
    GetSynchronizationSource()
    {
      return m_synchronizationSource;
    }

    void
    SetPacketId(unsigned int packetId)
    {
      m_packetId = packetId;
    }

    void
    SetTimestamp(unsigned long int timestamp)
    {
      m_timestamp = timestamp;
    }

    void
    SetSynchronizationSource(unsigned long int synchronizationSource)
    {
      m_synchronizationSource = synchronizationSource;
    }

  protected:
    /* The "classical" header is composed by 12 bytes. First 16 bits regard specific aspects of the
     * RTP behaviour, plus the payload type (last 7 bits). Then we have 16 bits used to carry
     * the packet ID, followed by 32 bits used for carrying the presentation timestamp.
     * Finally, additional 32 bits are used to store the ID of the flow. */
    unsigned int m_firstTwoBytes;
    unsigned int m_packetId;
    unsigned long int m_timestamp;
    unsigned long int m_synchronizationSource;

    /* Variable used to store the actual payload type, used to identify the payload of the packet by
     * the receiver. */
    RtpPayloadType m_payloadType;
  };

} // namespace ns3

#endif /* RTPPROTOCOL_H_ */
