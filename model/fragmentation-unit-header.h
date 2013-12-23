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

#ifndef FRAGMENTATIONUNITHEADER_H_
#define FRAGMENTATIONUNITHEADER_H_

#include "ns3/header.h"

namespace ns3
{

  class FragmentationUnitHeader : public Header
  {
  public:
    /* Declaration of the possible kinds of NAL fragmentation units. */
    enum FragmentationUnitType
    {
      UNSPECIFIED,
      START,
      END
    };

    /* Used at the receiver side */
    FragmentationUnitHeader()
    {
      m_isStart = false;
      m_isEnd = false;
      m_type = 0;
    }

    /* Used at the transmitter side */
    FragmentationUnitHeader(enum FragmentationUnitType startEndFlag, uint8_t nalType)
    {
      if (startEndFlag == START)
        {
          m_isStart = true;
          m_isEnd = false;
        }
      else if (startEndFlag == END)
        {
          m_isStart = false;
          m_isEnd = true;
        }
      else
        {
          /* Unspecified case, i.e., when the current NAL is neither the starting one,
           * nor the ending one. */
          m_isStart = false;
          m_isEnd = false;
        }

      m_type = (uint8_t) nalType;
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

    bool
    IsStart()
    {
      return m_isStart;
    }

    bool
    IsEnd()
    {
      return m_isEnd;
    }

  protected:
    bool m_isStart;
    bool m_isEnd;
    uint8_t m_type;
  };

} // namespace ns3

#endif /* FRAGMENTATIONUNITHEADER_H_ */
