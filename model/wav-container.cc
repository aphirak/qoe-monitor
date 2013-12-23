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

#include "wav-container.h"

namespace ns3
{

  WavContainer::WavContainer(std::string filename, Mode openMode, enum AVMediaType type) :
                             Container(filename, openMode, type)
  {
#if 0
    /* I fill the attributes with standard values */
    m_codecId = CODEC_ID_PCM_MULAW;
    m_sampleFormat = SAMPLE_FMT_S16;
    m_bitRate = 64000;
    m_sampleRate = 8000;
    m_channels = 1;
#endif

    m_fileOpen = false;
  }

  bool
  WavContainer::InitForWrite()
  {
    /* After the basic initialization, I must initialize the actual context */
    if (m_modeOfOperation == WRITE)
      {
        /* This means we need an output context */

        /* Now I must extract the information stored into the dataset to build the output format */
        // m_codedFileFormatContext = m_simulationDataset->GetAvFormatContext(); TODO USELESS

        /* Open the output format based on the outputFormatName variable */
        m_outputFormat = av_guess_format("wav", NULL, NULL);

        if (!m_outputFormat)
          {
            std::cout << "WavContainer: no suitable output format found\n";
            return false;
          }

        /* Create the output context */
        m_outputFormatContext = avformat_alloc_context();
        if (!m_outputFormatContext)
          {
            std::cout
                << "WavContainer: I could not open the required output format context\n";
            return false;
          }

        /* Assign the output format to the context */
        m_outputFormatContext->oformat = m_outputFormat;

        /* Assign the output name to the output context */
        snprintf(m_outputFormatContext->filename,
            sizeof(m_outputFormatContext->filename), "%s", m_filename.c_str());

        /* Now the output format is OK. I need to add a multimedia stream to it */
        AVStream* outputStream = av_new_stream(m_outputFormatContext, 1);

        // Check sul risultato
        if (!outputStream)
          {
            std::cout << "WavContainer: I could not allocate the stream\n";
            return false;
          }

        /* Start output stream configuration by means of STREAM COPY and CODEC CONTEXT COPY */
        if (avcodec_copy_context(outputStream->codec, &m_copyCodecContext) || !m_copyCodecContextAvailable)
          {
            std::cout << "WavContainer: Codec context copy error! Exit!\n";
            return false;
          }
        else
          {
            m_timeUnit = ((float) m_copyCodecContext.time_base.num) / (m_copyCodecContext.time_base.den);
          }

        if (m_copyStreamAvailable)
          {
            outputStream->codec->codec_id = m_copyStream.codec->codec_id;
            outputStream->codec->codec_type = m_copyStream.codec->codec_type;
            outputStream->codec->sample_fmt = m_copyStream.codec->sample_fmt;
            outputStream->codec->bit_rate = m_copyStream.codec->bit_rate;
            outputStream->codec->sample_rate = m_copyStream.codec->sample_rate;
            outputStream->codec->channels = m_copyStream.codec->channels;

            outputStream->pts = (AVFrac) {
                                          m_copyStream.pts.val,
                                          (int64_t) outputStream->codec->time_base.num,
                                          (int64_t) outputStream->codec->time_base.den
                                         };
            outputStream->codec->time_base = m_copyStream.time_base;
          }
        else
          {
            std::cout << "WavContainer: Stream context copy error! Exit!\n";
            return false;
          }

#if 0
        /* Start output stream configuration */
        outputStream->codec->codec_id = m_codecId;
        outputStream->codec->codec_type = m_codecType;
        outputStream->codec->sample_fmt = m_sampleFormat;
        outputStream->codec->bit_rate = m_bitRate;
        outputStream->codec->sample_rate = m_sampleRate;
        outputStream->codec->channels = m_channels;
#endif

        /* Check over the requirements for each specific format (see
         * row 84 of output-example.c */
        if (m_outputFormatContext->flags & AVFMT_GLOBALHEADER)
          {
            outputStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
          }

        /* Setup output parameters (see row 485 of output-example.c) */
        if (av_set_parameters(m_outputFormatContext, NULL) < 0)
          {
            std::cout << "WavContainer: invalid parameters\n";
            return false;
          }

        av_dump_format(m_outputFormatContext, 0, m_filename.c_str(), 1);

        if (!(m_outputFormat->flags & AVFMT_NOFILE))
          {
            if (url_fopen(&m_outputFormatContext->pb, m_filename.c_str(), AVIO_FLAG_WRITE)
                < 0)
              {
                std::cout << "WavContainer: Could not open output file\n";
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

  /* Function used to packetize from the data available inside the member queues */
  bool
  WavContainer::PacketizeFromQueue()
  {
    /* Starting from the information stored in the member queues, I have to align the
     * timestamps reported in each RTP packet with those required by the multimedia file
     * format. To do this, I dequeue ALL the packets stored in the RtpPacket queue, to extract
     * all the timestamps. */

    /* RtpPacket dequeue */
    while (m_rtpHeaderQueue.size() > 0)
      {
        unsigned int currentTimestamp =
            m_rtpHeaderQueue.front().GetPacketTimestamp();
        unsigned int packetSize = m_packetLengthQueue.front();

        m_rtpHeaderQueue.pop();
        m_packetLengthQueue.pop();

        for (unsigned int i = 0; i < packetSize; i++)
          {
            m_timestampQueue.push(currentTimestamp);
            currentTimestamp++;
          }
      }

    /* Now I should have a filled timestamp queue.
     * Now I have to verify if there is enough data to create an output frame */
    while (m_packetizationQueue.size() >= _WAV_FRAME_SIZE)
      {
        AVPacket outputFrame;
        av_init_packet(&outputFrame);

        outputFrame.size = _WAV_FRAME_SIZE;
        outputFrame.dts = m_timestampQueue.front();
        outputFrame.pts = outputFrame.dts;

        uint8_t tempBuffer[_WAV_FRAME_SIZE];

        for (unsigned int i = 0; i < _WAV_FRAME_SIZE; i++)
          {
            tempBuffer[i] = m_packetizationQueue.front();
            m_packetizationQueue.pop();
            m_timestampQueue.pop();
          }

        outputFrame.data = tempBuffer;

        /* Now I have to export the packet to the output context */
        if (av_interleaved_write_frame(m_outputFormatContext, &outputFrame)
            != 0)
          {
            std::cout << "WavContainer: Error while writing audio frame\n";
          }

        av_free_packet(&outputFrame);
      }

    return true;
  }

  /* This function will be called at the end of the transmission, to write the last bytes
   * to the file. However, this may be called also before, if necessary. */
  bool
  WavContainer::FinalizeFile()
  {
    /* First I must dequeue the last bytes stored inside the packetization queue */
    AVPacket outputFrame;
    av_init_packet(&outputFrame);

    unsigned int queueLength = m_packetizationQueue.size();
    outputFrame.size = queueLength;
    outputFrame.dts = m_timestampQueue.front();
    outputFrame.pts = m_timestampQueue.front();

    uint8_t* tempBuffer = (uint8_t*) calloc(queueLength, sizeof(uint8_t));
    assert(tempBuffer != NULL);

    for (unsigned int i = 0; i < queueLength; i++)
      {
        tempBuffer[i] = m_packetizationQueue.front();
        m_packetizationQueue.pop();
        m_timestampQueue.pop();
      }

    outputFrame.data = tempBuffer;

    /* Now I have to export the packet to the output context */
    if (av_interleaved_write_frame(m_outputFormatContext, &outputFrame) != 0)
      {
        std::cout << "WavContainer: Error while writing audio frame\n";
      }

    av_free_packet(&outputFrame);

    av_write_trailer(m_outputFormatContext);
    if (!(m_outputFormat->flags & AVFMT_NOFILE))
      {
        /* close the output file */
        url_fclose(m_outputFormatContext->pb);
        m_fileOpen = false;
      }

    free(tempBuffer);

    return true;
  }

}
