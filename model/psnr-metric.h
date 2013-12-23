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

#ifndef PSNR_METRIC_H_
#define PSNR_METRIC_H_

#include <cstdlib>
#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "metric.h"

namespace ns3
{
  class PsnrMetric : public /*ns3::*/Metric
  {
  public:

    PsnrMetric();

    typedef struct MetricRow
    {
      unsigned int m_frameNum;
      double m_psnrY;
      double m_psnrU;
      double m_psnrV;
    } MetricRow;

    virtual bool
    EvaluateQoe(std::string originalFilename, std::string receivedFilename);
    virtual bool
    PrintResults(std::string outputFilename, bool headers);
    double
    GetAverageYPsnr();
    double
    GetAverageUPsnr();
    double
    GetAverageVPsnr();

  private:
    unsigned int m_frameNumTot; //it counts the number of frames
    double m_avgY;
    double m_avgU;
    double m_avgV;

    std::vector<MetricRow> m_metric;

    double
    ComputePsnr(unsigned char* pFrame1, unsigned char* pFrame2, unsigned int width, unsigned int height);
  };
}

#endif /* PSNR_METRIC_H_ */
