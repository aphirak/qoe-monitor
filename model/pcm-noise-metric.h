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

#ifndef PCM_NOISE_METRIC_H_
#define PCM_NOISE_METRIC_H_

#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <sstream>
#include "ns3/wav-container.h"
#include "ns3/metric.h"

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

  /* This is the queue treshold used to trigger further push to the queue itself */
#define _QUEUE_TRESHOLD 1600

  class PcmNoiseMetric : public ns3::Metric
  {
  public:

    /* FIXME: I can't know the packet Id from the original and the received files only
     * (no packetization information is present!) */
    typedef struct MetricRow
    {
      unsigned long int m_timestamp;
      double m_noiseValue;
    } MetricRow;

    typedef enum ByteQueue
    {
      ORIGINAL, RECEIVED
    } ByteQueue;

    virtual bool
    EvaluateQoe(std::string originalFilename, std::string receivedFilename);
    virtual bool
    PrintResults(std::string outputFilename, bool headers);

  private:
    std::queue<uint8_t> m_originalFileQueue;
    std::queue<uint8_t> m_receivedFileQueue;
    std::queue<unsigned long int> m_originalTimestamp;

    std::vector<MetricRow> m_metric;

    bool
    FillQueue(WavContainer* container, ByteQueue byteQueue);
  };

}

#endif /* PCM_NOISE_METRIC_H_ */
