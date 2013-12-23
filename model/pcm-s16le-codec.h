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

#ifndef PCMS16LECODEC_H_
#define PCMS16LECODEC_H_

#include <string>
#include <cstdio>
#include <cstdlib>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>

#include "codec.h"

namespace ns3
{

  class PcmS16LeCodec : public ns3::Codec
  {
  private:
    /* List of attributes */
    string m_fileName;

  public:
    /* FIXME: formal parameter! */
    PcmS16LeCodec (string fileName);
    virtual ~PcmS16LeCodec ();

    /* FIXME Change the parameters! */
    void Encode ();
    void Decode ();
  };

}

#endif /* PCMS16LECODEC_H_ */
