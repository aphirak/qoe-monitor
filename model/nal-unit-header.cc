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

#include "nal-unit-header.h"

#define _NAL_UNIT_HEADER_DEBUG 0

namespace ns3
{

  TypeId
  NalUnitHeader::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::NalUnitHeader") .SetParent<Header> ();
    return tid;
  }

  TypeId
  NalUnitHeader::GetInstanceTypeId(void) const
  {
    return GetTypeId();
  }

  void
  NalUnitHeader::Print(std::ostream &os) const
  {
    os << "Nri: " << (unsigned int) m_nri << ", type: " << (unsigned int) m_type << "\n";
  }

  void
  NalUnitHeader::Serialize(Buffer::Iterator start) const
  {
    uint8_t serializedHeader = (m_nri << 5) | m_type;

    /* Actual write */
    start.Write(&serializedHeader, 1);
  }

  uint32_t
  NalUnitHeader::Deserialize(Buffer::Iterator start)
  {
    uint8_t deserializedHeader = 0;

    start.Read(&deserializedHeader, 1);

    m_type = deserializedHeader & 0x1F;
    m_nri = (deserializedHeader & 0x60) >> 5;

#if _NAL_UNIT_HEADER_DEBUG
    Print(std::cout);
#endif

    return GetSerializedSize();
  }

  uint32_t
  NalUnitHeader::GetSerializedSize() const
  {
    return 1;
  }

} // namespace ns3
