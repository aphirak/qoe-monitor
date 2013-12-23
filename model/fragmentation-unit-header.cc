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

#include "fragmentation-unit-header.h"

#define _FRAG_UNIT_HEADER_DEBUG 0

namespace ns3
{

  TypeId
  FragmentationUnitHeader::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::FragmentationUnitHeader") .SetParent<Header> ();
    return tid;
  }

  TypeId
  FragmentationUnitHeader::GetInstanceTypeId(void) const
  {
    return GetTypeId();
  }

  void
  FragmentationUnitHeader::Print(std::ostream &os) const
  {
    if (m_isStart)
      os << "First fragmentation unit, ";
    if (m_isEnd)
      os << "Last fragmentation unit, ";
    if (!m_isStart && !m_isEnd)
      os << "Framgentation unit, ";

    os << "type: " << (unsigned int) m_type << "\n";
  }

  void
  FragmentationUnitHeader::Serialize(Buffer::Iterator start) const
  {
    uint8_t serializedHeader = m_type;

    if (m_isStart)
      serializedHeader |= 0x80;
    else if (m_isEnd)
      serializedHeader |= 0x40;

    /* Actual write */
    start.Write(&serializedHeader, 1);
  }

  uint32_t
  FragmentationUnitHeader::Deserialize(Buffer::Iterator start)
  {
    uint8_t deserializedHeader = 0;

    start.Read(&deserializedHeader, 1);

    m_type = deserializedHeader & 0x1F;

    if (deserializedHeader & 0x80)
      m_isStart = true;
    else if (deserializedHeader & 0x40)
      m_isEnd = true;

#if _FRAG_UNIT_HEADER_DEBUG
    Print(std::cout);
#endif

    return GetSerializedSize();
  }

  uint32_t
  FragmentationUnitHeader::GetSerializedSize() const
  {
    return 1;
  }

} // namespace ns3
