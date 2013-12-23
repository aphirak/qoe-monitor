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

#ifndef CONTAINER_H_
#define CONTAINER_H_

#include <queue>
#include "ns3/rtp-protocol.h"

#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
}
#endif

namespace ns3
{

  class Container
  {

  public:

    enum Mode
    {
      READ, WRITE
    };

    Container(std::string filename, enum Mode openMode, enum AVMediaType type);

    virtual
    ~Container();

    /* Method used to initialize the class for reading purposes */
    bool
    InitForRead();

    virtual bool
    InitForWrite() = 0;

    /* Method used to set to the container a new data packet */
    void
    SetNextPacket(RtpProtocol rtpHeader, uint8_t* packetData,
                  unsigned int length);

    /* Method used to get from the container a new data packet */
    virtual bool
    GetNextPacket(AVPacket* readFrame);

    virtual bool
    FinalizeFile() = 0;

    /* Generic getter/setter methods */
    void
    SetCodecId(enum CodecID codecId);
    void
    SetCodecType(enum AVMediaType codecType);
    void
    SetSampleFormat(enum SampleFormat format);
    void
    SetBitRate(int bitRate);
    void
    SetSampleRate(int sampleRate);
    void
    SetChannels(int channels);

    /* Method used to copy the codec context from that used at the sender side */
    void
    SetCodecContext(AVCodecContext copyContext);
    void
    SetStream(AVStream copyStream);

    enum CodecID
    GetCodecId();
    enum AVMediaType
    GetCodecType();
    enum SampleFormat
    GetSampleFormat();
    int
    GetBitRate();
    int
    GetSampleRate();
    float
    GetSamplingInterval();
    int
    GetChannels();
    AVCodecContext
    GetCodecContext();
    AVStream
    GetStream();

  protected:
    /* Filename used for input/output */
    std::string m_filename;

    /* Flag used to assess whether the file is open or not */
    bool m_fileOpen;

    /* Useful contexts */
    AVFormatContext* m_inputFormatContext;
    AVFormatContext* m_outputFormatContext;
    AVOutputFormat* m_outputFormat;

    /* Used for reading the context, e.g., when opening
     * a file in reading mode, to extract the codec context
     * that has to be copied to the target receiving context. */
    AVCodecContext m_inputCodecContext;

    /* Copy context, used to store the sender-side context that has to
     * be used at receiving side to "clone" the codec's settings. */
    AVCodecContext m_copyCodecContext;

    /* Stream used to copy the information from the reading stream for further
     * possible use (e.g., to reconstruct the same stream at the receiver side */
    AVStream m_copyStream;

    /* Flags used to determine if the copies are available or not */
    bool m_copyCodecContextAvailable;
    bool m_copyStreamAvailable;

    /* Mode of operation - read or write */
    enum Mode m_modeOfOperation;

    /* A Rtp header queue, used to store all the information regarding
     * every received packet */
    std::queue<RtpProtocol> m_rtpHeaderQueue;

    /* A byte queue exploitable by the re-packetization process */
    std::queue<uint8_t> m_packetizationQueue;

    /* A queue used to store the lengths of the packets received */
    std::queue<unsigned int> m_packetLengthQueue;

    virtual bool
    PacketizeFromQueue() = 0;

    /* Generic container/codec variables */
    float m_timeUnit;
    int m_streamNumber;

    enum CodecID m_codecId;
    enum AVMediaType m_codecType;
    enum SampleFormat m_sampleFormat;
    int m_bitRate;
    int m_sampleRate;
    int m_channels;
  };

}

#endif /* CONTAINER_H_ */
