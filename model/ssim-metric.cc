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

#include "ssim-metric.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

namespace ns3
{
  SsimMetric::SsimMetric()
  {
    m_frameNumTot = 0;
    m_avgSsim = 0;
  }

  double
  SsimMetric::GetAverageSsim()
  {
    return m_avgSsim;
  }

  bool
  SsimMetric::EvaluateQoe(std::string origFilename,/*originalFilename,*/
      std::string recvFilename)/*receivedFilename)*/
  {
    const char * originalFilename = origFilename.c_str();
    const char * receivedFilename = recvFilename.c_str();

    unsigned int size = 0;
    unsigned int width = 352; //default width if not specified
    unsigned int height = 288; //default height if not specified
    unsigned int yuvFormat = 420; //default yuv format

    //compute the frame size according to the yuvFormat value
    if(yuvFormat == 420)       size = width*height*3/2;
    else if(yuvFormat == 400)  size = width*height;
    else if(yuvFormat == 422)  size = width*height*2;
    else if(yuvFormat == 444)  size = width*height*3;

    double ssim_frame; //SSIM of single frame

    FILE * originalFile, * receivedFile;

    //open the original video file
    if ((originalFile = fopen(originalFilename, "rb")) == NULL)
      {
        std::cout << "Errore nell'apertura del file:" << originalFilename << "!\n";
        return false;
      }

    //open the received video file
    if ((receivedFile = fopen(receivedFilename, "rb")) == NULL)
      {
        std::cout << "Errore nell'apertura del file:" << receivedFilename << "!\n";
        return false;
      }

    unsigned char * originalFrame = (unsigned char*) calloc(size, sizeof(unsigned char));
    assert(originalFrame != NULL);

    unsigned char * receivedFrame = (unsigned char*) calloc(size, sizeof(unsigned char));
    assert(receivedFrame != NULL);

    for(;;) //infinite cicle to read until the end of the file
      {
        //read the frame from the original file
        if(1 != (fread (originalFrame, size, 1, originalFile)))
          break;

        //read the frame from the received file
        if(1 != (fread (receivedFrame, size, 1, receivedFile)))
          break;

        //count the frame's number
        m_frameNumTot++;

        //compute ssim metric of the current frame
        ssim_frame = ComputeSsim(originalFrame, receivedFrame, width, height);

        MetricRow currentRow;

        //fill the metric row
        currentRow.m_frameNum=m_frameNumTot;
        currentRow.m_ssim=ssim_frame;

        //put the currentRow into the result vector
        m_metric.push_back(currentRow);

        //sum the current ssim value in order to compute the average ssim value at the end
        m_avgSsim += ssim_frame;
      }

    // Compute the average SSIM
    m_avgSsim /= m_frameNumTot;

    //close the files
    fclose(originalFile);
    fclose(receivedFile);

    //free the memory allocated for the frames
    free(originalFrame);
    free(receivedFrame);

    return true;
  }

  /******************************* SSIM metric **************************************/

  /*
   * this function computes ssim metric for each frame
   * */
  double
  SsimMetric::ComputeSsim(uint8_t *origFrame, uint8_t *recvFrame, int width, int height)
  {
    double ssim_frame, ssim_window = 0.0;
    int windowDim = 8; //window dimension
    uint8_t **pOrigFrame, **pRecvFrame;
    double origMean, recvMean, origVariance, recvVariance, covariance;

    //now load the refFrame and the compFrame in a matrix
    pOrigFrame = LoadFrame(origFrame, width, height); //loading of reference frame
    pRecvFrame = LoadFrame(recvFrame, width, height); //loading of compare frame

    //to move the sliding window pixel by pixel
    int row=0;

    //now apply the sliding window of windowDim size and compute ssim
    for(int col=0; col<width-windowDim+1; col++)
      {
        //now compute the mean value
        origMean = MeanSlidingWindow(pOrigFrame, windowDim, col, row);
        recvMean = MeanSlidingWindow(pRecvFrame, windowDim, col, row);

        //now compute the variance
        origVariance = VarianceSlidingWindow(pOrigFrame, origMean, windowDim, col, row);
        recvVariance = VarianceSlidingWindow(pRecvFrame, recvMean, windowDim, col, row);

        //now compute the covariance
        covariance = CovarianceSlidingWindow(pOrigFrame, origMean, pRecvFrame, recvMean, windowDim, col, row);

        //now compute the ssim
        ssim_window += SsimSlidingWindow(origMean, recvMean, origVariance, recvVariance, covariance);

        //to move the sliding window pixel by pixel
        if(col+1==width-windowDim+1 && row+1<height-windowDim+1)
          {
            col=-1;
            row++;
          }
      }

    ssim_frame = ssim_window/((width-windowDim+1)*(height-windowDim+1));

    return ssim_frame;

    for ( int h=0; h < height ; h++ )
      delete pOrigFrame [h];
    delete pOrigFrame;
    for ( int h=0; h < height ; h++ )
      delete pRecvFrame [h];
    delete pRecvFrame;
  }

  /*
   * This function load a frame in a matrix
   * */
  uint8_t**
  SsimMetric::LoadFrame(uint8_t *p, int cols, int rows)
  {
    uint8_t **frame = new uint8_t*[rows];
    for(int r=0; r<rows; r++)
      frame[r] = new uint8_t[cols]; //allocazione dinamica matrice

    int frameSize = cols*rows;

    int i=0, j=0;
    for(int k=0; k<frameSize; k++)
      {
        frame[i][j]=p[k];

        if(i<rows && j==cols-1)
          {
            j=0;
            i++;
          }
        else
          {
            j++;
          }
      }

    return frame;
  }

  /*
   * this function computes the mean in the sliding window
   * */
  double
  SsimMetric::MeanSlidingWindow(uint8_t **p, int wDim, int col, int row)
  {
    double mean;
    long int sum = 0;

    //printf("NUOVA MEDIA\n");

    for(int i=0+row; i<wDim+row; i++)
      for(int j=0+col; j<wDim+col; j++)
        {
          sum += p[i][j];
          //printf("SUM: %ld ---------- p[%d][%d]: %d\n", sum, i, j, p[i][j]);
          //printf("%d\n", p[i][j]);
        }
    //printf("Sum prima finestra 8x8: %ld\n", sum);
    mean = (double)sum/(wDim * wDim);
    //printf("Media prima finestra %dx%d: %f\n", wDim, wDim, mean);

    return mean;
  }

  /*
   * this function computes the variance in the sliding window
   * */
  double
  SsimMetric::VarianceSlidingWindow(uint8_t **p, double mean, int wDim, int col, int row)
  {
    long int sum = 0;
    double var;

    for(int i=0+row; i<wDim+row; i++)
      for(int j=0+col; j<wDim+col; j++)
        {
          sum += (p[i][j]-mean)*(p[i][j]-mean);
          //printf("SUM: %ld ---------- p[%d][%d]: %d\n", sum, i, j, p[i][j]);
          //printf("%d\n", p[i][j]);
        }
    //var = (double)sum/(wDim * wDim);
    var = (double)sum/((wDim * wDim)-1);
    //printf("Varianza prima finestra %dx%d: %f\n", wDim, wDim, var);

    return var;
  }

  /*
   * this function computes the covariance in the sliding window
   * */
  double
  SsimMetric::CovarianceSlidingWindow(uint8_t **pOrig, double origMean, uint8_t **pRecv, double recvMean, int wDim, int col, int row)
  {
    long int sum = 0;
    double cov;

    for(int i=0+row; i<wDim+row; i++)
      for(int j=0+col; j<wDim+col; j++)
        {
          sum += (pOrig[i][j]-origMean)*(pRecv[i][j]-recvMean);
          //printf("SUM: %ld ---------- p[%d][%d]: %d\n", sum, i, j, p[i][j]);
          //printf("%d\n", p[i][j]);
        }
    //cov = (double)sum/(wDim * wDim);
    cov = (double)sum/((wDim * wDim)-1);
    //printf("Covarianza prima finestra %dx%d: %f\n", wDim, wDim, cov);

    return cov;
  }

  /*
   * this function computes the ssim in the sliding window
   * */
  double
  SsimMetric::SsimSlidingWindow(double origMean, double recvMean, double origVar, double recvVar, double cov)
  {
    double ssim;

    ssim = (double)((2*origMean*recvMean + C1)*(2*cov + C2))/((origMean*origMean + recvMean*recvMean + C1)*(origVar + recvVar + C2));
    //printf("SSIM prima finestra 8x8: %f\n", ssim);

    return ssim;
  }

  /******************************* end SSIM metric **************************************/

  /*
   * This function prints the results in a file
   * */
  bool
  SsimMetric::PrintResults(std::string outputFilename, bool headers)
  {
    std::string output = outputFilename + "_ssim.csv";
    const char * outFilename = output.c_str();

    FILE * outputFile;

    //open the result file in write mode
    if((outputFile = fopen(outFilename, "w+")) == NULL)
      {
        std::cout << "Errore nell'apertura del file:" << outFilename << "!\n";
        return false;
      }

    //output trace print
    if (headers)
      {
        fprintf(outputFile,"FrameNUM, SSIM\n");
      }

    //read all of the result rows and write each one into the result file
    std::vector<MetricRow>::iterator iterator;
    for (iterator = m_metric.begin(); iterator < m_metric.end(); iterator++)
      {
        fprintf(outputFile,"%d,%f\n", iterator->m_frameNum, iterator->m_ssim);
      }

    //close the result file
    fclose(outputFile);

    return true;
  }

}
