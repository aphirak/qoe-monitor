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

#ifndef SIMULATIONDATASET_H_
#define SIMULATIONDATASET_H_

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "packet-trace-structure.h"

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

  class SimulationDataset
  {
  public:
    /* Enum used to set the type of file used for the simulation
     * FIXME: maybe useless */
    enum FileType
    {
      AUDIO, VIDEO, MIX
    };

    SimulationDataset();

    void
    SetFileType(enum FileType type);

    FileType
    GetFileType();

    /* File methods */
    void
    SetOriginalRawFile(std::string filename);
    std::string
    GetOriginalRawFile();
    void
    SetOriginalCodedFile(std::string filename);
    std::string
    GetOriginalCodedFile();
    void
    SetReceivedRawFile(std::string filename);
    std::string
    GetReceivedRawFile();
    void
    SetReceivedCodedFile(std::string filename);
    std::string
    GetReceivedCodedFile();
    void
    SetReceivedReconstructedFile(std::string filename);
    std::string
    GetReceivedReconstructedFile();
    void
    SetTraceFileId(std::string filename);

    /* Trace structures methods */
    void
    PushBackPacketTraceRow(PacketTraceRow traceRow);
    std::vector<PacketTraceRow>
    GetPacketTrace();
    void
    PushBackSenderTraceRow(SenderTraceRow traceRow);
    std::vector<SenderTraceRow>
    GetSenderTrace();
    void
    PushBackReceiverTraceRow(ReceiverTraceRow traceRow);
    std::vector<ReceiverTraceRow>
    GetReceiverTrace();
    void
    PushBackJitterTraceRow(JitterTraceRow traceRow);
    std::vector<JitterTraceRow>
    GetJitterTrace();

    /* Trace print methods
     * If true is passed, this will print also the headers describing the content
     * of the trace file. */
    void
    PrintTraces(bool headers);
    void
    PrintTracesToVideo(bool headers);

    unsigned int
    GetPacketSent();
    unsigned int
    GetPacketReceived();

    void
    SetSamplingInterval(float samplingInterval);
    float
    GetSamplingInterval();

  protected:
    // TODO: check the class' attributes
    std::string m_originalRawFile;
    std::string m_originalCodedFile;
    std::string m_receivedRawFile;
    std::string m_receivedCodedFile;
    std::string m_receivedReconstructedFile;
    std::string m_traceFileId;

    // FIXME: Do I need one of them for each receiver?
    // FIXME: Are they pointers or are they used directly by codec and packetizer?
    //        Now are used as variables, because we use copy constructor.
    std::vector<PacketTraceRow> m_packetTrace;
    std::vector<SenderTraceRow> m_senderTrace;
    std::vector<ReceiverTraceRow> m_receiverTrace;
    std::vector<JitterTraceRow> m_jitterTrace;

    /* Variables used to store some useful statistics */
    unsigned int m_packetSent;
    unsigned int m_packetReceived;

    /* The sampling interval */
    float m_samplingInterval;
    enum FileType m_fileType;
  };

} // namespace ns3

#endif /* SIMULATIONDATASET_H_ */
