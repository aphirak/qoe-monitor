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

#ifndef MPEG4_CONTAINER_H_
#define MPEG4_CONTAINER_H_

#include <cstdlib>
#include <cassert>
#include "container.h"

namespace ns3
{

  class Mpeg4Container : public ns3::Container
  {
  public:
    Mpeg4Container(std::string filename, enum Mode openMode, enum AVMediaType type);

    virtual bool
    InitForWrite();

    /* Methods inherited from Container that have to be implemented. */
    virtual bool
    FinalizeFile();
  protected:
    virtual bool
    PacketizeFromQueue();

    unsigned long int m_lastPts;
  };

}

#endif /* MPEG4_CONTAINER_H_ */
