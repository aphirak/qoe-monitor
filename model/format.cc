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

#include "format.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>

#define _FORMAT_DEBUG 0

namespace ns3
{

  Format::Format(SimulationDataset* simulationDataset,
      std::string encodingOptions, std::string decodingOptions,
      float samplingInterval)
  {
    /* In this case the mode will be equal to 1 */
    m_mode = 1;

    /* Now I extract every filename from the SimulationDataset structure */
    m_originalRawFilename = simulationDataset->GetOriginalRawFile();
    m_originalCodedFilename = simulationDataset->GetOriginalCodedFile();
    m_receivedRawFilename = simulationDataset->GetReceivedRawFile();
    m_receivedCodedFilename = simulationDataset->GetReceivedCodedFile();

    /* I save the encoding/decoding options required by the user */
    m_ffmpegEncodingOptions = encodingOptions;
    m_ffmpegDecodingOptions = decodingOptions;

    m_simulationDataset = simulationDataset;

    m_simulationDataset->SetSamplingInterval(samplingInterval);
  }

  Format::~Format()
  {
    // TODO Auto-generated destructor stub
  }

  int
  Format::EncodeAndFormatFile()
  {
    int returnValue = 0;

    /* Check the mode of operation */
    /* FIXME: only mode = 1 is implemented */
    if (m_mode == 1)
      {
        /* Now I have to call the external ffmpeg tool
         * The typical sintax of ffmpeg is:
         * ffmpeg -i <inputfile> -f <format> -acodec <codec> <outputfile>
         */
        std::stringstream command;
        command << "ffmpeg -i " << m_originalRawFilename << " "
            << m_ffmpegEncodingOptions << " " << m_originalCodedFilename;

#if _FORMAT_DEBUG
        std::cout << "External ENCODING command string: ";
        std::cout << command.str() << "\n";
#endif

        /* I execute the command */
        returnValue = system(command.str().c_str());

#if _FORMAT_DEBUG
        std::cout << "Return value: ";
        std::cout << returnValue << "\n";
#endif
      }

    return returnValue;
  }

  int
  Format::DecodeAndFormatFile()
  {
    int returnValue = 0;

    /* Check the mode of operation */
    if (m_mode == 1)
      {
        /* Now I have to call the external ffmpeg tool
         * The typical sintax of ffmpeg is:
         * ffmpeg -i <inputfile> -f <format> -acodec <codec> <outputfile>
         */
        std::stringstream command;
        command << "ffmpeg -i " << m_receivedCodedFilename << " "
            << m_ffmpegDecodingOptions << " " << m_receivedRawFilename;

#if _FORMAT_DEBUG
        std::cout << "External DECODING command string: ";
        std::cout << command.str() << "\n";
#endif

        /* I execute the command */
        returnValue = system(command.str().c_str());

#if _FORMAT_DEBUG
        std::cout << "Return value: ";
        std::cout << returnValue << "\n";
#endif        
      }
    return returnValue;
  }

}
