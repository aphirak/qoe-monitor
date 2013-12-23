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

#ifndef WAVCONTAINER_H_
#define WAVCONTAINER_H_

#include <cstdlib>
#include <cassert>
#include "container.h"

namespace ns3
{

  /* FIXME */
#define _WAV_FRAME_SIZE 1600

  class WavContainer : public ns3::Container
  {
  public:
    WavContainer(std::string filename, Mode openMode, enum AVMediaType type);

    virtual bool
    InitForWrite();

    virtual bool
    FinalizeFile();

  protected:
    std::queue<unsigned long int> m_timestampQueue;

    virtual bool
    PacketizeFromQueue();
  };

}

#endif /* WAVCONTAINER_H_ */
