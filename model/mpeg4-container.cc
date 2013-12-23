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

#include "mpeg4-container.h"

namespace ns3
{

  Mpeg4Container::Mpeg4Container(std::string filename, enum Mode openMode, enum AVMediaType type) :
                                 Container(filename, openMode, type)
  {
    /* TODO: Fill here the parameters needed for standard operation */
    m_copyCodecContextAvailable = false;
    m_copyStreamAvailable = false;
    m_lastPts = 0;
  }

  bool
  Mpeg4Container::InitForWrite()
  {
    /* After the basic initialization, I must initialize the actual context */
    if (m_modeOfOperation == WRITE &&
        m_copyCodecContextAvailable &&
        m_copyStreamAvailable)
      {
        /* This means we need an output context */

        /* Open the output format based on the outputFormatName variable */
        m_outputFormat = av_guess_format("mp4", NULL, NULL);

        if (!m_outputFormat)
          {
            std::cout << "Mpeg4Container: no suitable output format found\n";
            return false;
          }

        m_outputFormat->video_codec = m_copyCodecContext.codec_id;

        /* Create the output context */
        m_outputFormatContext = avformat_alloc_context();
        if (!m_outputFormatContext)
          {
            std::cout << "Mpeg4Container: I could not open the required output format context\n";
            return false;
          }

        /* Assign the output format to the context */
        m_outputFormatContext->oformat = m_outputFormat;

        /* Assign the output name to the output context */
        snprintf(m_outputFormatContext->filename,
            sizeof(m_outputFormatContext->filename), "%s", m_filename.c_str());

        /* Now the output format is OK. I need to add a multimedia stream to it */
        AVStream* outputStream = av_new_stream(m_outputFormatContext, 0);

        // Check sul risultato
        if (!outputStream)
          {
            std::cout << "Mpeg4Container: I could not allocate the stream\n";
            return false;
          }

        /* Start output stream configuration by means of STREAM COPY and CODEC CONTEXT COPY */
        if (avcodec_copy_context(outputStream->codec, &m_copyCodecContext))
          {
            std::cout << "Mpeg4Container: Codec context copy error! Exit!\n";
            return false;
          }

        outputStream->pts = (AVFrac) {
                                      m_copyStream.pts.val,
                                      (int64_t) outputStream->codec->time_base.num,
                                      (int64_t) outputStream->codec->time_base.den
                                     };

        outputStream->codec->time_base = m_copyStream.time_base;
        outputStream->sample_aspect_ratio = m_copyStream.sample_aspect_ratio;
        outputStream->codec->codec_tag = 0;

        /* Set the actual time-base unit */
        m_timeUnit = ((float) m_copyCodecContext.time_base.num) / (m_copyCodecContext.time_base.den);

        /* Check over the requirements for each specific format (see
         * row 84 of output-example.c */
        if (m_outputFormatContext->flags & AVFMT_GLOBALHEADER)
          {
            outputStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
          }

        /* Setup output parameters (see row 485 of output-example.c) */
        if (av_set_parameters(m_outputFormatContext, NULL) < 0)
          {
            std::cout << "Mpeg4Container: invalid parameters\n";
            return false;
          }

        av_dump_format(m_outputFormatContext, 0, m_filename.c_str(), 1);

        if (!(m_outputFormat->flags & AVFMT_NOFILE))
          {
            if (avio_open(&m_outputFormatContext->pb, m_filename.c_str(), AVIO_FLAG_WRITE)
                < 0)
              {
                std::cout << "Mpeg4Container: Could not open output file\n";
                return false;
              }
          }

        m_fileOpen = true;

        /* Write output file header */
        av_write_header(m_outputFormatContext);

        return true;
      }

    return false;
  }

  bool
  Mpeg4Container::PacketizeFromQueue()
  {
    /* As for Mpeg4, it is possible to dequeue exactly one packet per round */
    unsigned int packetSize = m_packetLengthQueue.front();
    unsigned int currentTimestamp = m_rtpHeaderQueue.front().GetPacketTimestamp();

    m_rtpHeaderQueue.pop();
    m_packetLengthQueue.pop();

    /* Now I should have a packetization queue of packetSize length */
    AVPacket outputFrame;
    av_init_packet(&outputFrame);

    outputFrame.size = packetSize;
    outputFrame.dts = currentTimestamp;
    outputFrame.pts = currentTimestamp;

    // Duration can be set to 0 if unknown
    outputFrame.duration = 0;
    outputFrame.pos = -1;

    uint8_t* tempBuffer = (uint8_t*) calloc(packetSize, sizeof(uint8_t));
    assert(tempBuffer != NULL);

    for (unsigned int i = 0; i < packetSize; i++)
      {
        tempBuffer[i] = m_packetizationQueue.front();
        m_packetizationQueue.pop();
      }

    outputFrame.data = tempBuffer;

    /* Now I have to export the packet to the output context */
    // if (av_interleaved_write_frame(m_outputFormatContext, &outputFrame) != 0)
    if (av_write_frame(m_outputFormatContext, &outputFrame) != 0)
      {
        std::cout << "Mpeg4Container: Error while writing video frame\n";
      }

    av_free_packet(&outputFrame);
    free(tempBuffer);

    return true;
  }

  bool
  Mpeg4Container::FinalizeFile()
  {
    av_write_trailer(m_outputFormatContext);
    if (!(m_outputFormat->flags & AVFMT_NOFILE))
      {
        /* close the output file */
        avio_close(m_outputFormatContext->pb);
        m_fileOpen = false;
      }

    return true;
  }

}
