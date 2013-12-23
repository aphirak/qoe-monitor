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

#include "pcm-noise-metric.h"

namespace ns3
{

  bool
  PcmNoiseMetric::EvaluateQoe(std::string originalFilename,
      std::string receivedFilename)
  {
    /* The idea is to fill both queues with data coming from the two files. When enough
     * data is present, the comparison will start.
     * However, it is important to underline that the received file could be smaller
     * than the original one, because the simulation may last only a fraction of the whole
     * duration of the Pcm file. Then, it's necessary to clearly understand when the received
     * file has already been completely read, to stop the read process over the original file.
     */

    /* First I have to open the two files */
    WavContainer originalFile(originalFilename, WavContainer::READ, AVMEDIA_TYPE_AUDIO);
    WavContainer receivedFile(receivedFilename, WavContainer::READ, AVMEDIA_TYPE_AUDIO);

    originalFile.InitForRead();
    receivedFile.InitForRead();

    bool goOn = true;
    do
      {
        /* Then I must fill the received file's queue */
        if (!FillQueue(&receivedFile, RECEIVED))
          {
            /* EOC has been reached */
            goOn = false;
          }

        if (!FillQueue(&originalFile, ORIGINAL))
          {
            /* A more detailed check has to be done */
            if (m_originalFileQueue.size() < m_receivedFileQueue.size())
              {
                std::cout
                    << "PcmNoiseMetric: Error: original file is smaller than the received one!\n";
                return false;
              }
          }

        /* Now I have to compare the data stored in the buffers */
        for (unsigned int i = 0; i < m_receivedFileQueue.size(); i++)
          {
            char originalSample = m_originalFileQueue.front();
            char receivedSample = m_receivedFileQueue.front();
            unsigned long int currentTimestamp = m_originalTimestamp.front();

            m_originalFileQueue.pop();
            m_receivedFileQueue.pop();
            m_originalTimestamp.pop();

            MetricRow currentRow;

            /* FIXME: Currently the metric is related only to a simple difference between the samples */
            currentRow.m_noiseValue = ((double) originalSample)
                - receivedSample;
            currentRow.m_timestamp = currentTimestamp;

            m_metric.push_back(currentRow);
          }

      }
    while (goOn);

    return true;
  }

  bool
  PcmNoiseMetric::FillQueue(WavContainer* container, ByteQueue byteQueue)
  {
    std::queue<uint8_t>* currentQueue;

    if (byteQueue == ORIGINAL)
      {
        currentQueue = &m_originalFileQueue;
      }
    else if (byteQueue == RECEIVED)
      {
        currentQueue = &m_receivedFileQueue;
      }

    AVPacket packet;
    unsigned long int timestamp = 0;
    while (currentQueue->size() < _QUEUE_TRESHOLD)
      {
        if (container->GetNextPacket(&packet))
          {
            /* Fill the proper queue */
            if (byteQueue == ORIGINAL)
              {
                /* I extract also the presentation timestamp */
                timestamp = packet.pts;
              }

            for (int i = 0; i < packet.size; i++)
              {
                if (byteQueue == ORIGINAL)
                  {
                    m_originalTimestamp.push(timestamp);
                    timestamp++;
                  }

                currentQueue->push(packet.data[i]);
              }
          }
        else
          {
            /* EOF has been reached */
            return false;
          }
      }

    return true;
  }

  bool
  PcmNoiseMetric::PrintResults(std::string outputFilename, bool headers)
  {
    std::stringstream output;
    output << outputFilename << "-noise-metric.csv";

    std::fstream outputFile;
    outputFile.open(output.str().c_str(), std::ios::out);

    /* Output trace print */
    if (headers)
      {
        outputFile << "timestamp, noise\n";
      }

    std::vector<MetricRow>::iterator iterator;
    for (iterator = m_metric.begin(); iterator < m_metric.end(); iterator++)
      {
        outputFile << iterator->m_timestamp << ", " << iterator->m_noiseValue
            << "\n";
      }

    outputFile.close();
    return true;
  }

}
