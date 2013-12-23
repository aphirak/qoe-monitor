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

#ifndef SSIM_METRIC_H_
#define SSIM_METRIC_H_

#include <cstdlib>
#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include "metric.h"

#define C1 6.5025
#define C2 58.5225

namespace ns3
{

  class SsimMetric : public /*ns3::*/Metric
  {
  public:
    SsimMetric();

    typedef struct MetricRow
    {
      unsigned int m_frameNum;
      double m_ssim;
    } MetricRow;

    virtual bool
    EvaluateQoe(std::string originalFilename, std::string receivedFilename);
    virtual bool
    PrintResults(std::string outputFilename, bool headers);
    double
    GetAverageSsim();

  private:
    unsigned int m_frameNumTot;
    double m_avgSsim;

    std::vector<MetricRow> m_metric;

    double
    ComputeSsim(uint8_t *origFrame, uint8_t *recvFrame, int width, int height);

    uint8_t**
    LoadFrame(uint8_t *p, int cols, int rows);

    double
    MeanSlidingWindow(uint8_t **p, int wDim, int col, int row);

    double
    VarianceSlidingWindow(uint8_t **p, double mean, int wDim, int col, int row);

    double
    CovarianceSlidingWindow(uint8_t **pOrig, double origMean, uint8_t **pRecv, double recvMean, int wDim, int col, int row);

    double
    SsimSlidingWindow(double origMean, double recvMean, double origVar, double recvVar, double cov);
  };
}

#endif /* SSIM_METRIC_H_ */
