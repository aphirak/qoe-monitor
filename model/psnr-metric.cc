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

#include "psnr-metric.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

namespace ns3
{
  PsnrMetric::PsnrMetric()
  {
    m_frameNumTot = 0;
    m_avgY = 0;
    m_avgU = 0;
    m_avgV = 0;
  }

  double
  PsnrMetric::GetAverageYPsnr()
  {
    return m_avgY;
  }

  double
  PsnrMetric::GetAverageUPsnr()
  {
    return m_avgU;
  }

  double
  PsnrMetric::GetAverageVPsnr()
  {
    return m_avgV;
  }

  bool
  PsnrMetric::EvaluateQoe(std::string origFilename,/*originalFilename,*/
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

    double psnrY, psnrU, psnrV; //PSNR of components Y, U and V

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

    unsigned char * originalFrame = (unsigned char*)calloc(size, sizeof(unsigned char));
    assert(originalFrame != NULL);

    unsigned char * originalFrameY = originalFrame;
    unsigned char * originalFrameU = originalFrame + width*height;
    unsigned char * originalFrameV = originalFrame + width*height*5/4;

    unsigned char * receivedFrame = (unsigned char*)calloc(size, sizeof(unsigned char));
    assert(receivedFrame != NULL);

    unsigned char * receivedFrameY = receivedFrame;
    unsigned char * receivedFrameU = receivedFrame + width*height;
    unsigned char * receivedFrameV = receivedFrame + width*height*5/4;

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

        //compute psnr metric for Y, U and V components of the current frame
        psnrY = ComputePsnr(originalFrameY, receivedFrameY, width, height);
        psnrU = ComputePsnr(originalFrameU, receivedFrameU, width/2, height/2);
        psnrV = ComputePsnr(originalFrameV, receivedFrameV, width/2, height/2);

        MetricRow currentRow;

        //fill the metric row
        currentRow.m_frameNum=m_frameNumTot;
        currentRow.m_psnrY=psnrY;
        currentRow.m_psnrU=psnrU;
        currentRow.m_psnrV=psnrV;

        //put the currentRow into the result vector
        m_metric.push_back(currentRow);

        //sum the current psnr value in order to compute the average psnr value at the end
        m_avgY+=psnrY;
        m_avgU+=psnrU;
        m_avgV+=psnrV;
      }

    // Compute each average PSNR
    m_avgY /= m_frameNumTot;
    m_avgU /= m_frameNumTot;
    m_avgV /= m_frameNumTot;

    //close the files
    fclose(originalFile);
    fclose(receivedFile);

    //free the memory allocated for the frames
    free(originalFrame);
    free(receivedFrame);

    return true;
  }


  /******************************* PSNR metric **************************************/

  /*
   * This function computes the PSNR value between two frames
   * */
  double
  PsnrMetric::ComputePsnr(unsigned char* pFrame1, unsigned char* pFrame2, unsigned int width, unsigned int height)
  {
    unsigned int size = width*height;
    double mse; //Mean Square Error
    long long diff;
    long long diffQuad = 0;
    double PSNR = 0.0;
    unsigned int max = 255; //if the image is at n=8 bits per pixel => max=(2^n)-1

    for (unsigned int i=0; i<width; i++)
      {
        for(unsigned int j=0; j<height; j++)
          {
            diff = (*pFrame1++) - (*pFrame2++); //compute the difference of the pixels value between the two frames
            diffQuad += diff * diff; //compute the square
          }
      }

    mse = diffQuad/size; //compute the MSE

    if(mse!=0)
      PSNR = 20 * log10(max/sqrt(mse)); //compute the PSNR
    else //in order to prevent the division by 0
      PSNR = 99.0;

    return PSNR;
  }

  /******************************* end PSNR metric **************************************/

  /*
   * This function prints the results in a file
   * */
  bool
  PsnrMetric::PrintResults(std::string outputFilename, bool headers)
  {
    std::string output = outputFilename + "_psnr.csv";
    const char * outFilename = output.c_str();

    FILE * outputFile;

    //open the result file in write mode
    if((outputFile = fopen(outFilename, "w+")) == NULL)
      {
        std::cout << "Errore nell'apertura del file:" << outFilename << "!\n";
        return false;
      }

    // output trace print
    if (headers)
      {
        fprintf(outputFile,"FrameNUM, PSNR_Y, PSNR_U, PSNR_V\n");
      }

    //read all of the result rows and write each one into the result file
    std::vector<MetricRow>::iterator iterator;
    for (iterator = m_metric.begin(); iterator < m_metric.end(); iterator++)
      {
        fprintf(outputFile,"%d,%f,%f,%f\n", iterator->m_frameNum, iterator->m_psnrY, iterator->m_psnrU, iterator->m_psnrV);
      }

    //close the result file
    fclose(outputFile);

    return true;
  }

}
