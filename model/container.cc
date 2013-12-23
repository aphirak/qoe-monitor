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

#include "container.h"

#define _CONTAINER_DEBUG 0

namespace ns3
{

  Container::Container(std::string filename, enum Mode openMode, enum AVMediaType type) :
                       m_rtpHeaderQueue(),
                       m_packetizationQueue(),
                       m_packetLengthQueue()
  {
    m_filename = filename;
    m_modeOfOperation = openMode;

    m_fileOpen = false;

    m_inputFormatContext = NULL;
    m_outputFormatContext = NULL;
    m_outputFormat = NULL;

    m_codecId = CODEC_ID_NONE;
    m_codecType = type;
    m_sampleFormat = SAMPLE_FMT_NONE;
    m_bitRate = 0;
    m_sampleRate = 0;
    m_channels = 0;

    m_timeUnit = 0.0;
    m_streamNumber = -1;
  }

  Container::~Container()
  {
    /* The file needs to be closed, if it is not
     * FIXME: verify if this has to be done also for the writing process */
    if (m_fileOpen && m_modeOfOperation == READ)
      {
        m_fileOpen = false;
        av_close_input_file(m_inputFormatContext);
      }
    else if (m_fileOpen && m_modeOfOperation == WRITE)
      {
        m_fileOpen = false;
        av_close_input_file(m_outputFormatContext); // Even if looks wrong, the function is that, indeed.
      }
  }

  /* Method used to setup the reading process from the multimedia file */
  bool
  Container::InitForRead()
  {
    /* Initialize each output format */
    av_register_all();

    /* Open input file - FIXME: deprecated */
    int ret = 0;
    m_inputFormatContext = NULL;
    if ((ret = avformat_open_input(&m_inputFormatContext, m_filename.c_str(),
         NULL, NULL)) < 0)
      {
        std::cout << "Container: Cannot open input file\n";
        return false;
      }
    else
      {
        m_fileOpen = true;
      }

    /* Let's find out stream's information */
    if ((ret = av_find_stream_info(m_inputFormatContext)) < 0)
      {
        std::cout << "Container: Cannot find stream information\n";
        return false;
      }

    /* Extract the audio stream's index */
    m_streamNumber = -1;
    for (unsigned int i = 0; i < m_inputFormatContext->nb_streams; i++)
      {
        if (m_inputFormatContext->streams[i]->codec->codec_type
            == m_codecType)
          {
            m_streamNumber = i;
            break;
          }
      }

    if (m_streamNumber == -1)
      {
        std::cout << "Container: Cannot find audio stream\n";
        return false;
      }

    /* Calculate and store the sampling interval */
    m_timeUnit = ((float) m_inputFormatContext->streams[m_streamNumber]->time_base.num)
               / m_inputFormatContext->streams[m_streamNumber]->time_base.den;

    /* Set the sample rate */
    m_sampleRate = 1/m_timeUnit;

    /* CodecContext extraction */
    m_inputCodecContext = *(m_inputFormatContext->streams[m_streamNumber]->codec);

    /* Stream extraction for possible further usage */
    m_copyStream = *(m_inputFormatContext->streams[m_streamNumber]);

    return true;
  }

  /* Function used to read the next packet from the underlying container */
  bool
  Container::GetNextPacket(AVPacket* readFrame)
  {
    if (m_modeOfOperation == WRITE)
      {
        /* This method should return nothing in this case */
        return false;
      }

    /* A new frame has to be read from the file */
    if (m_inputFormatContext->streams == 0)
      {
#if _CONTAINER_DEBUG
        std::cout << "Container: streams pointer equal to 0!\n";
#endif
        return false;
      }

    int returnCode = av_read_frame(m_inputFormatContext, readFrame);
    if (returnCode < 0)
      {
        /* EOF has been reached
         * The file needs to be closed */
        m_fileOpen = false;
        av_close_input_file(m_inputFormatContext);

        return false;
      }
    return true;
  }

  void
  Container::SetNextPacket(RtpProtocol rtpHeader, uint8_t* packetData,
                           unsigned int length)
  {
    /* At first, I fill the queues with fresh data */
    m_rtpHeaderQueue.push(rtpHeader);
    m_packetLengthQueue.push(length);

    for (unsigned int i = 0; i < length; i++)
      {
        m_packetizationQueue.push(packetData[i]);
      }

    /* Now I call the specific (i.e., virtual) packetization function */
    PacketizeFromQueue();
  }

  void
  Container::SetCodecContext(AVCodecContext copyContext)
  {
    m_copyCodecContext = copyContext;
    m_copyCodecContextAvailable = true;
  }

  void
  Container::SetStream(AVStream copyStream)
  {
    m_copyStream = copyStream;
    m_copyStreamAvailable = true;
  }

  void
  Container::SetCodecId(enum CodecID codecId)
  {
    m_codecId = codecId;
  }

  void
  Container::SetCodecType(enum AVMediaType codecType)
  {
    m_codecType = codecType;
  }

  void
  Container::SetSampleFormat(enum SampleFormat format)
  {
    m_sampleFormat = format;
  }

  void
  Container::SetBitRate(int bitRate)
  {
    m_bitRate = bitRate;
  }

  void
  Container::SetSampleRate(int sampleRate)
  {
    m_sampleRate = sampleRate;
  }

  void
  Container::SetChannels(int channels)
  {
    m_channels = channels;
  }

  enum CodecID
  Container::GetCodecId()
  {
    return m_codecId;
  }

  enum AVMediaType
  Container::GetCodecType()
  {
    return m_codecType;
  }

  enum SampleFormat
  Container::GetSampleFormat()
  {
    return m_sampleFormat;
  }

  int
  Container::GetBitRate()
  {
    return m_bitRate;
  }

  int
  Container::GetSampleRate()
  {
    return m_sampleRate;
  }

  float
  Container::GetSamplingInterval()
  {
    // return ((float) 1) / m_sampleRate; // FIXME
    return m_timeUnit;
  }

  int
  Container::GetChannels()
  {
    return m_channels;
  }

  AVCodecContext
  Container::GetCodecContext()
  {
    return m_inputCodecContext;
  }

  AVStream
  Container::GetStream()
  {
    return m_copyStream;
  }
}
