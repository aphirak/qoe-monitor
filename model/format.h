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

#ifndef FORMAT_H_
#define FORMAT_H_

#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
}
#endif

#include <string>
#include "simulation-dataset.h"

namespace ns3
{

  class Format
  {
  protected:
    /* The mode variable determines the behaviour of the object.
     * If it is put equal to 0, then the codec class will be used; otherwise
     * the following transcoding methods simply call the external ffmpeg tool
     * with the proper parameters. */
    unsigned int m_mode;
    SimulationDataset* m_simulationDataset;
    std::string m_originalRawFilename;
    std::string m_originalCodedFilename;
    std::string m_receivedRawFilename;
    std::string m_receivedCodedFilename;
    std::string m_ffmpegEncodingOptions;
    std::string m_ffmpegDecodingOptions;

    bool
    ExtractFormatInformation();

  public:
    // FIXME: Currently there is a single constructor!
    Format(SimulationDataset* simulationDataset, std::string encodingOptions,
        std::string decodingOptions, float samplingInterval);

    virtual
    ~Format();

    virtual int
    EncodeAndFormatFile();
    /* FIXME: Ideally I should avoid the use of a filename here. This has been
     * considered only for initial tests */
    virtual int
    DecodeAndFormatFile();
  };

}

#endif /* FORMAT_H_ */
