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
 *
 **************************************************************************************************
 *
 * Description: the network consists of a couple of nodes, connected by means of a
 * point-to-point link at 2 Mb/s (with 2 ms delay). The first node (n.0) represents
 * the sender, while the second one (n.1) acts as a receiver.
 *
 *         Node 0 -------------------------- Node 1
 *                      2 Mb/s, 2 ms
 *
 * By providing as input an h264-encoded mp4 file (i.e., codec = h264, container = mpeg4),
 * the sender is able to stream its content to the receiver.
 *
 * Please note that, at least for now, only h264 encoded files with STRICTLY MONOTONICAL pts/dts
 * can be employed with this script. It is possible to convert a generic h264 video with:
 *
 *      ffmpeg -i <input-file> -vcodec libx264 -preset ultrafast -profile baseline
 *             -tune zerolatency <output-file.mp4>
 *
 * Then, by enabling enablePsnr and/or enableSsim flags, it is possible to decide which
 * metric(s) has to be used for the evaluation.
 *
 * The output files consists of csv files regarding jitter estimation, psnr and/or ssim
 * values.
 */

#include "ns3/point-to-point-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/simulation-dataset.h"
#include "ns3/h264-packetizer.h"
#include "ns3/multimedia-application-sender.h"
#include "ns3/multimedia-application-receiver.h"
#include "ns3/multimedia-file-rebuilder.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/error-model.h"
#include "ns3/mpeg4-container.h"
#include "ns3/psnr-metric.h"
#include "ns3/ssim-metric.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"

#include <iostream>
#include <cstring>
#include <cassert>

using namespace ns3;

/* Simple FFMpeg wrapper for encoding/decoding purposes */
int Ffmpeg(std::string inputFileName, std::string outputFileName)
{
  std::stringstream command;
  command << "ffmpeg -i " << inputFileName
          << " " << outputFileName;

  /* I execute the command */
  int returnCode = system(command.str().c_str());
  if (returnCode != 0)
    {
      std::cout << "Error while calling FFMpeg, abort.\n";
    }
  return returnCode;
}

int
main(int argc, char *argv[])
{
  /* Some simulation variables */
  double packetLossRate = 0.001;
  unsigned int mtu = 1400;

  bool enablePsnr = true;
  bool enableSsim = false;

  /* Command line argument check */
  if (argc != 2)
    {
      std::cout << "Wrong number of arguments.\n";
      std::cout << "Usage: " << std::string(argv[0]) << " <input-264-file.mp4>\n";
      exit(1);
    }

  std::string jitterBufferLength("20ms");
  std::string simulationDuration("200s");
  std::string transmitterStartTime("2s");
  std::string transmitterStopTime("100s");
  std::string receiverStartTime("1s");
  std::string receiverStopTime("101s");

  /* Extract the coded filename */
  std::string codedFilename(argv[1]);

  /* Filenames creation */
  size_t extPosition = codedFilename.rfind(".mp4");
  std::string fileIdentifier = codedFilename.substr(0, extPosition);

  std::string rawFilename = fileIdentifier + ".yuv";
  std::string receivedFilename = fileIdentifier + ".received.mp4";
  std::string receivedRawFilename = fileIdentifier + ".received.yuv";

  std::string traceFileID = fileIdentifier + "-trace";
  std::string metricFile = fileIdentifier + "-metric";

  /* QoE monitor setup */
  SimulationDataset* dataset = new SimulationDataset();

  dataset->SetOriginalRawFile(rawFilename);
  dataset->SetOriginalCodedFile(codedFilename);
  dataset->SetReceivedCodedFile(receivedFilename);
  dataset->SetReceivedReconstructedFile(receivedRawFilename);
  dataset->SetTraceFileId(traceFileID);

  Mpeg4Container mpeg4ReadingContainer = Mpeg4Container(dataset->GetOriginalCodedFile(),
                                                        Container::READ, AVMEDIA_TYPE_VIDEO);
  mpeg4ReadingContainer.InitForRead();
  dataset->SetSamplingInterval(mpeg4ReadingContainer.GetSamplingInterval());

  H264Packetizer videoPacketizer(mtu, dataset);

  /* Network setup */
  NodeContainer nodes;
  nodes.Create(2);

  /* Error model setup */
  Config::SetDefault("ns3::RateErrorModel::ErrorRate", DoubleValue(packetLossRate));
  Config::SetDefault("ns3::RateErrorModel::ErrorUnit", StringValue("ERROR_UNIT_PACKET"));

  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
//  em->SetRandomVariable(RandomVariable(UniformVariable(0.0, 1.0)));

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install(nodes);

  PointToPointNetDevice* receiverDevice =
        dynamic_cast<PointToPointNetDevice*> (PeekPointer(devices.Get(1)));
  receiverDevice->SetReceiveErrorModel(em);

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  /* Receiver side application setup
   * First I create an mpeg4 container for the received file (output) */
  Mpeg4Container mpeg4Container = Mpeg4Container(dataset->GetReceivedCodedFile(),
                                                 Container::WRITE, AVMEDIA_TYPE_VIDEO);

  /* Now I take the codec context from the reading context (input) and I copy
   * its settings to the output codec context */
  mpeg4Container.SetCodecContext(mpeg4ReadingContainer.GetCodecContext());

  /* I copy the stream's settings from the reading context (input) to the output context */
  mpeg4Container.SetStream(mpeg4ReadingContainer.GetStream());

  /* Now I can initialize the output container for writing */
  mpeg4Container.InitForWrite();

  /* Create the receiver's multimedia file rebuilder */
  MultimediaFileRebuilder fileRebuilder(&mpeg4Container, dataset);

  /* Create the receiver's side application */
  Ptr<MultimediaApplicationReceiver> multimediaReceiver = CreateObject<
      MultimediaApplicationReceiver> (nodes.Get(1), dataset, Time(std::string(jitterBufferLength)));

  multimediaReceiver->SetupReceiverPort(400);
  multimediaReceiver->SetupFileRebuilder(&fileRebuilder);

  nodes.Get(1)->AddApplication(multimediaReceiver);

  multimediaReceiver->SetStartTime(Time(receiverStartTime));
  multimediaReceiver->SetStopTime(Time(receiverStopTime));

  /* Create the sender's side application */
  Ptr<MultimediaApplicationSender> multimediaSender = CreateObject<MultimediaApplicationSender>
                                                      (nodes.Get(0), &videoPacketizer, dataset);

  multimediaSender->SetupDestinationAddress(interfaces.GetAddress(1), 400);
  nodes.Get(0)->AddApplication(multimediaSender);

  multimediaSender->SetStartTime(Time(transmitterStartTime));
  multimediaSender->SetStopTime(Time(transmitterStopTime));

  std::cout << "Starting simulation\n";

  Simulator::Stop (Time(simulationDuration));
  Simulator::Run();

  /* Print traces to file */
  dataset->PrintTraces(true);

  /* Decoding received file */
  std::cout << "Received file decoding with FFMpeg...\n";
  assert(Ffmpeg(receivedFilename, receivedRawFilename) == 0);

  /* Decode the original h264 file to perform video comparison */
  std::cout << "Original YUV (raw) file decoding with FFMpeg...\n";
  assert(Ffmpeg(codedFilename, rawFilename) == 0);

  if (enablePsnr)
    {
      /* Computing PSNR */
      std::cout << "PSNR computing...";
      std::cout.flush();

      PsnrMetric psnr;
      psnr.EvaluateQoe(rawFilename, receivedRawFilename);

      /* Print the metric output without any header */
      psnr.PrintResults(metricFile.c_str(), false);
      std::cout << " done!\n";
    }

  if (enableSsim)
    {
      /* Computing SSIM */
      std::cout << "SSIM computing...";
      std::cout.flush();

      SsimMetric ssim;
      ssim.EvaluateQoe(rawFilename, receivedRawFilename);

      /* Print the metric output without any header */
      ssim.PrintResults(metricFile.c_str(), false);
      std::cout << " done!\n";
    }

  delete dataset;
  Simulator::Destroy();
  return 0;
}
