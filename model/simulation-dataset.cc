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

// FIXME: maybe this file is useless, because of the definition of each function
// in the header file itself.

#include "simulation-dataset.h"

namespace ns3
{

  SimulationDataset::SimulationDataset()
  {
    m_packetSent = 0;
    m_packetReceived = 0;
    m_samplingInterval = 0;

    /* As default, the file type is VIDEO */
    m_fileType = VIDEO;
  }

  void
  SimulationDataset::SetFileType(SimulationDataset::FileType type)
  {
    m_fileType = type;
  }

  SimulationDataset::FileType
  SimulationDataset::GetFileType()
  {
    return m_fileType;
  }

  void
  SimulationDataset::SetOriginalRawFile(std::string filename)
  {
    m_originalRawFile = filename;
  }

  std::string
  SimulationDataset::GetOriginalRawFile()
  {
    return m_originalRawFile;
  }

  void
  SimulationDataset::SetOriginalCodedFile(std::string filename)
  {
    m_originalCodedFile = filename;
  }

  std::string
  SimulationDataset::GetOriginalCodedFile()
  {
    return m_originalCodedFile;
  }

  void
  SimulationDataset::SetReceivedRawFile(std::string filename)
  {
    m_receivedRawFile = filename;
  }

  std::string
  SimulationDataset::GetReceivedRawFile()
  {
    return m_receivedRawFile;
  }

  void
  SimulationDataset::SetReceivedCodedFile(std::string filename)
  {
    m_receivedCodedFile = filename;
  }

  std::string
  SimulationDataset::GetReceivedCodedFile()
  {
    return m_receivedCodedFile;
  }

  void
  SimulationDataset::SetReceivedReconstructedFile(std::string filename)
  {
    m_receivedReconstructedFile = filename;
  }

  std::string
  SimulationDataset::GetReceivedReconstructedFile()
  {
    return m_receivedReconstructedFile;
  }

  void
  SimulationDataset::SetTraceFileId(std::string filename)
  {
    m_traceFileId = filename;
  }

  void
  SimulationDataset::PushBackPacketTraceRow(PacketTraceRow traceRow)
  {
    m_packetTrace.push_back(traceRow);
  }

  std::vector<PacketTraceRow>
  SimulationDataset::GetPacketTrace()
  {
    return m_packetTrace;
  }

  void
  SimulationDataset::PushBackSenderTraceRow(SenderTraceRow traceRow)
  {
    m_senderTrace.push_back(traceRow);

    /* I also increase by one the number of packet sent */
    m_packetSent++;
  }

  std::vector<SenderTraceRow>
  SimulationDataset::GetSenderTrace()
  {
    return m_senderTrace;
  }

  void
  SimulationDataset::PushBackReceiverTraceRow(ReceiverTraceRow traceRow)
  {
    m_receiverTrace.push_back(traceRow);

    /* I also increase by one the number of packet received */
    m_packetReceived++;
  }

  std::vector<ReceiverTraceRow>
  SimulationDataset::GetReceiverTrace()
  {
    return m_receiverTrace;
  }

  void
  SimulationDataset::PushBackJitterTraceRow(JitterTraceRow traceRow)
  {
    m_jitterTrace.push_back(traceRow);
  }

  std::vector<JitterTraceRow>
  SimulationDataset::GetJitterTrace()
  {
    return m_jitterTrace;
  }

  void
  SimulationDataset::PrintTraces(bool headers)
  {
    /* Define filenames */
    std::stringstream packetTraceFilename, senderTraceFilename,
        receiverTraceFilename, jitterTraceFilename;

    packetTraceFilename << m_traceFileId << "-packet.csv";
    senderTraceFilename << m_traceFileId << "-sender.csv";
    receiverTraceFilename << m_traceFileId << "-receiver.csv";
    jitterTraceFilename << m_traceFileId << "-jitter.csv";

    /* Open output streams
     * FIXME: no check has been performed! Assert everything! */
    std::fstream packetTraceFile, senderTraceFile, receiverTraceFile,
        jitterTraceFile;

    packetTraceFile.open(packetTraceFilename.str().c_str(), std::ios::out);
    senderTraceFile.open(senderTraceFilename.str().c_str(), std::ios::out);
    receiverTraceFile.open(receiverTraceFilename.str().c_str(), std::ios::out);
    jitterTraceFile.open(jitterTraceFilename.str().c_str(), std::ios::out);

    /* If "headers" is true I print also the header of each file */
    if (headers)
      {
        packetTraceFile << "packetId, size, decoding-timestamp, "
                        << "presentation-timestamp, rtp-timestamp\n";
        senderTraceFile << "packetId, timestamp\n";
        receiverTraceFile << "packetId, timestamp\n";
        jitterTraceFile << "receptionTime, packetId, jitter\n";
      }

    /* Packet trace print */
    std::vector<PacketTraceRow>::iterator ptIterator;
    for (ptIterator = m_packetTrace.begin(); ptIterator < m_packetTrace.end(); ptIterator++)
      {
        packetTraceFile << ptIterator->m_packetId << ", "
            << ptIterator->m_packetSize << ", "
            << ptIterator->m_decodingTimestamp << ", "
            << ptIterator->m_playbackTimestamp << ", "
            << ptIterator->m_rtpTimestamp << "\n";
      }

    /* Sender trace print */
    std::vector<SenderTraceRow>::iterator stIterator;
    for (stIterator = m_senderTrace.begin(); stIterator < m_senderTrace.end(); stIterator++)
      {
        senderTraceFile << stIterator->m_packetId << ", "
            << stIterator->m_senderTimestamp << "\n";
      }

    /* Receiver trace print */
    std::vector<ReceiverTraceRow>::iterator rtIterator;
    for (rtIterator = m_receiverTrace.begin(); rtIterator
        < m_receiverTrace.end(); rtIterator++)
      {
        receiverTraceFile << rtIterator->m_packetId << ", "
            << rtIterator->m_receiverTimestamp << "\n";
      }

    /* Jitter trace print */
    std::vector<JitterTraceRow>::iterator jIterator;
    for (jIterator = m_jitterTrace.begin(); jIterator < m_jitterTrace.end(); jIterator++)
      {
        jitterTraceFile << jIterator->m_receptionTime << ", " << jIterator->m_packetId << ", " 
                        << jIterator->m_jitter << "\n";
      }

    /* Close output streams */
    packetTraceFile.close();
    senderTraceFile.close();
    receiverTraceFile.close();
    jitterTraceFile.close();
  }

  void
  SimulationDataset::PrintTracesToVideo(bool headers)
  {
    /* Packet trace print */
    if (headers)
      {
        std::cout << "Packet trace: packetId, size, decoding-timestamp, "
                  << "presentation-timestamp, rtp-timestamp\n";
      }

    std::vector<PacketTraceRow>::iterator ptIterator;
    for (ptIterator = m_packetTrace.begin(); ptIterator < m_packetTrace.end(); ptIterator++)
      {
        std::cout << ptIterator->m_packetId << ", " << ptIterator->m_packetSize
            << ", " << ptIterator->m_decodingTimestamp << ", "
            << ", " << ptIterator->m_playbackTimestamp << ", "
            << ptIterator->m_rtpTimestamp << "\n";
      }

    /* Sender trace print */
    if (headers)
      {
        std::cout << "Sender trace: packetId, timestamp\n";
      }

    std::vector<SenderTraceRow>::iterator stIterator;
    for (stIterator = m_senderTrace.begin(); stIterator < m_senderTrace.end(); stIterator++)
      {
        std::cout << stIterator->m_packetId << ", "
            << stIterator->m_senderTimestamp << "\n";
      }

    /* Receiver trace print */
    if (headers)
      {
        std::cout << "Receiver trace: packetId, timestamp\n";
      }

    std::vector<ReceiverTraceRow>::iterator rtIterator;
    for (rtIterator = m_receiverTrace.begin(); rtIterator
        < m_receiverTrace.end(); rtIterator++)
      {
        std::cout << rtIterator->m_packetId << ", "
            << rtIterator->m_receiverTimestamp << "\n";
      }

    /* Jitter trace print */
    if (headers)
      {
        std::cout << "Jitter trace: packetId, jitter\n";
      }

    std::vector<JitterTraceRow>::iterator jIterator;
    for (jIterator = m_jitterTrace.begin(); jIterator < m_jitterTrace.end(); jIterator++)
      {
        std::cout << jIterator->m_receptionTime << ", " << jIterator->m_packetId << ", " 
                  << jIterator->m_jitter << "\n";
      }
  }

  unsigned int
  SimulationDataset::GetPacketSent()
  {
    return m_packetSent;
  }

  unsigned int
  SimulationDataset::GetPacketReceived()
  {
    return m_packetReceived;
  }

  void
  SimulationDataset::SetSamplingInterval(float samplingInterval)
  {
    m_samplingInterval = samplingInterval;
  }

  float
  SimulationDataset::GetSamplingInterval()
  {
    return m_samplingInterval;
  }

} // namespace ns3
