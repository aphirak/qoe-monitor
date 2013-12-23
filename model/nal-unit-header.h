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

#ifndef NALUNITHEADER_H_
#define NALUNITHEADER_H_

#include "ns3/header.h"

namespace ns3
{

  class NalUnitHeader : public Header
  {
  public:
    /* Declaration of the possible kinds of NAL units.
     * FIXME: currently only two types are supported, i.e., NAL-unit
     * and FU-A. */
    enum NalUnitType
    {
      RESERVED = 0,
      NAL_UNIT = 23,
      FU_A = 28
    };

    /* Used at the receiver side */
    NalUnitHeader()
    {
      m_nri = 0;
      m_type = 0;
    }

    /* Used at the transmitter side */
    NalUnitHeader(uint8_t nri, enum NalUnitType type)
    {
      m_nri = nri;
      m_type = (uint8_t) type;
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

    uint8_t
    GetType()
    {
      return m_type;
    }

    uint8_t
    GetNri()
    {
      return m_nri;
    }

  protected:
    uint8_t m_nri;
    uint8_t m_type;
  };

} // namespace ns3

#endif /* NALUNITHEADER_H_ */
