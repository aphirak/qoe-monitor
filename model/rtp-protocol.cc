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

#include "rtp-protocol.h"

namespace ns3
{

  TypeId
  RtpProtocol::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::RtpProtocol") .SetParent<Header> ();
    return tid;
  }

  TypeId
  RtpProtocol::GetInstanceTypeId(void) const
  {
    return GetTypeId();
  }

  void
  RtpProtocol::Print(std::ostream &os) const
  {
    os << "First two bytes: " << m_firstTwoBytes << ", " << "Packet ID: "
        << m_packetId << ", " << "Timestamp: " << m_timestamp << ", "
        << "Sync. source: " << m_synchronizationSource << "\n";
  }

  void
  RtpProtocol::Serialize(Buffer::Iterator start) const
  {
    /* Temporary buffer used to setup the whole header */
    uint8_t serializedHeader[12];

    serializedHeader[0] = m_firstTwoBytes >> 8;
    serializedHeader[1] = m_firstTwoBytes & 0x00FF;
    serializedHeader[2] = m_packetId >> 8;
    serializedHeader[3] = m_packetId & 0x00FF;
    serializedHeader[4] = m_timestamp >> 24;
    serializedHeader[5] = (m_timestamp >> 16) & 0x00FF;
    serializedHeader[6] = (m_timestamp >> 8) & 0x00FF;
    serializedHeader[7] = m_timestamp & 0x00FF;
    serializedHeader[8] = m_synchronizationSource >> 24;
    serializedHeader[9] = (m_synchronizationSource >> 16) & 0x00FF;
    serializedHeader[10] = (m_synchronizationSource >> 8) & 0x00FF;
    serializedHeader[11] = m_synchronizationSource & 0x00FF;

    /* Actual write */
    start.Write(serializedHeader, 12);
  }

  uint32_t
  RtpProtocol::Deserialize(Buffer::Iterator start)
  {
    /* Temporary buffer used to setup the whole header */
    uint8_t deserializedHeader[12];

    start.Read(deserializedHeader, 12);

    m_firstTwoBytes = (deserializedHeader[0] << 8) | deserializedHeader[1];
    m_packetId = (deserializedHeader[2] << 8) | deserializedHeader[3];
    m_timestamp = (deserializedHeader[4] << 24) | (deserializedHeader[5] << 16)
        | (deserializedHeader[6] << 8) | deserializedHeader[7];

    m_synchronizationSource = (deserializedHeader[8] << 24)
        | (deserializedHeader[9] << 16) | (deserializedHeader[10] << 8)
        | deserializedHeader[11];

    return GetSerializedSize();
  }

  uint32_t
  RtpProtocol::GetSerializedSize() const
  {
    /* FIXME: this holds true only for "classical" RTP, without extensions or anything else */
    return 12;
  }

} // namespace ns3
