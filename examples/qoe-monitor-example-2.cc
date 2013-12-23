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
 * Description: the network consists of 6 nodes, according to the following sketch:
 *
 *    Node 0 (video sender)                   Node 4 (video receiver)
 *        \                                    /
 *         \                                  /
 *          \                                /
 *          Node 2 -------------------- Node 3
 *          /                                \
 *         /                                  \
 *        /                                    \
 *    Node 1 (cross traffic sender)           Node 5 (cross traffic receiver)
 *
 * Node 0 hosts a video sender, while Node 4 hosts the corresponding receiver. Similarly,
 * Node 1 hosts a simple cross traffic source, while Node 5 hosts the corresponding receiver.
 * All links have a capacity of 2 Mb/s and a delay of 2 ms.
 *
 * By providing as input an h264-encoded mp4 file (i.e., codec = h264, container = mpeg4),
 * the sender is able to stream its content to the receiver. With this setup it's possible
 * to assess the transmission performance in presence of a cross traffic source.
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

#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/simulation-dataset.h"
#include "ns3/h264-packetizer.h"
#include "ns3/multimedia-application-sender.h"
#include "ns3/multimedia-application-receiver.h"
#include "ns3/multimedia-file-rebuilder.h"
#include "ns3/error-model.h"
#include "ns3/mpeg4-container.h"
#include "ns3/psnr-metric.h"
#include "ns3/ssim-metric.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/applications-module.h"

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
  /* Command line argument check */
  if (argc != 2)
    {
      std::cout << "Wrong number of arguments.\n";
      std::cout << "Usage: " << std::string(argv[0]) << " <input-264-file.mp4>\n";
      exit(1);
    }

  /* Some simulation variables */
  unsigned int mtu = 1400;

  bool enablePsnr = true;
  bool enableSsim = false;

  bool enableCrossTraffic = true;
  bool isTcpCrossTraffic = false;

  std::string jitterBufferLength("20ms");
  std::string simulationDuration("200s");
  std::string transmitterStartTime("2s");
  std::string transmitterStopTime("100s");
  std::string receiverStartTime("1s");
  std::string receiverStopTime("101s");

  /* Cross-traffic settings */
  float ctInterPacketTime = 0.0066;
  unsigned int ctPacketSize = 500;
  unsigned int ctMaxPackets = 10000000;
  std::string ctSenderStartTime("10s");
  std::string ctSenderStopTime("40s");
  std::string ctReceiverStartTime("10s");
  std::string ctReceiverStopTime("50s");

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
  nodes.Create(6);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  /* Devices creation for each point-to-point link */
  NetDeviceContainer devices[5];

  devices[0] = pointToPoint.Install(nodes.Get(0), nodes.Get(2));
  devices[1] = pointToPoint.Install(nodes.Get(1), nodes.Get(2));
  devices[2] = pointToPoint.Install(nodes.Get(2), nodes.Get(3));
  devices[3] = pointToPoint.Install(nodes.Get(3), nodes.Get(4));
  devices[4] = pointToPoint.Install(nodes.Get(3), nodes.Get(5));

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.252");

  Ipv4InterfaceContainer interfaces[5];

  for(unsigned int i = 0; i < 5; i++)
    {
      interfaces[i] = address.Assign(devices[i]);
      address.NewNetwork();
    }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

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
      MultimediaApplicationReceiver> (nodes.Get(4), dataset, Time(std::string(jitterBufferLength)));

  multimediaReceiver->SetupReceiverPort(400);
  multimediaReceiver->SetupFileRebuilder(&fileRebuilder);

  nodes.Get(4)->AddApplication(multimediaReceiver);

  multimediaReceiver->SetStartTime(Time(receiverStartTime));
  multimediaReceiver->SetStopTime(Time(receiverStopTime));

  /* Create the sender's side application */
  Ptr<MultimediaApplicationSender> multimediaSender = CreateObject<MultimediaApplicationSender>
                                                      (nodes.Get(0), &videoPacketizer, dataset);

  multimediaSender->SetupDestinationAddress(interfaces[3].GetAddress(1), 400);
  nodes.Get(0)->AddApplication(multimediaSender);

  multimediaSender->SetStartTime(Time(transmitterStartTime));
  multimediaSender->SetStopTime(Time(transmitterStopTime));

  if (enableCrossTraffic)
    {
      /* Cross-traffic application setup */
      ApplicationContainer ctSenderContainer, ctReceiverContainer;

      if (isTcpCrossTraffic)
        {
          /* Sender side */
          BulkSendHelper ctTcpSender("ns3::TcpSocketFactory",
                                     InetSocketAddress(interfaces[4].GetAddress(1), 500));
          ctTcpSender.SetAttribute("MaxBytes", UintegerValue(0));
          ctSenderContainer = ctTcpSender.Install(nodes.Get(1));

          /* Receiver side */
          PacketSinkHelper ctTcpReceiver("ns3::TcpSocketFactory",
                                         InetSocketAddress (Ipv4Address::GetAny (), 500));
          ctReceiverContainer = ctTcpReceiver.Install(nodes.Get(5));
        }
      else
        {
          /* Sender side */
          UdpClientHelper ctUdpSender(interfaces[4].GetAddress(1), 500);

          ctUdpSender.SetAttribute("MaxPackets", UintegerValue(ctMaxPackets));
          ctUdpSender.SetAttribute("Interval", TimeValue(Seconds(ctInterPacketTime)));
          ctUdpSender.SetAttribute("PacketSize", UintegerValue(ctPacketSize));

          ctSenderContainer = ctUdpSender.Install(nodes.Get(1));

          /* Receiver side */
          Address ctReceiverAddress = InetSocketAddress(interfaces[4].GetAddress(1), 500);
          PacketSinkHelper ctUdpReceiver("ns3::UdpSocketFactory", ctReceiverAddress);
          ctReceiverContainer = ctUdpReceiver.Install(nodes.Get(5));
        }

      ctSenderContainer.Start(Time(ctSenderStartTime));
      ctSenderContainer.Stop(Time(ctSenderStopTime));

      ctReceiverContainer.Start(Time(ctReceiverStartTime));
      ctReceiverContainer.Stop(Time(ctReceiverStopTime));
    }

  /* Flow monitor setup */
  FlowMonitorHelper flowmon_helper;
  Ptr<FlowMonitor> monitor = flowmon_helper.InstallAll();
  monitor->SetAttribute("DelayBinWidth", ns3::DoubleValue(0.0005));
  monitor->SetAttribute("JitterBinWidth", ns3::DoubleValue(0.0005));
  monitor->SetAttribute("PacketSizeBinWidth", ns3::DoubleValue(20));

  /* Actual simulation start */
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

  /* Flow monitor post-processing */
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon_helper.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iterator = stats.begin (); iterator != stats.end (); iterator++)
    {
      /* Extract the classifier info. for the current flow */
      Ipv4FlowClassifier::FiveTuple tuple = classifier->FindFlow (iterator->first);

      uint16_t destinationPort = tuple.destinationPort;
      uint16_t sourcePort = tuple.sourcePort;
      Ipv4Address sourceAddress = tuple.sourceAddress;
      Ipv4Address destinationAddress = tuple.destinationAddress;

      /* I extract loss, throughput (in bps), delay, jitter and packet size */
      double senderStreamDuration = (iterator->second.timeLastTxPacket - iterator->second.timeFirstTxPacket).GetSeconds();
      double receiverStreamDuration = (iterator->second.timeLastRxPacket - iterator->second.timeFirstRxPacket).GetSeconds();
      double sampleLoss = (iterator->second.txPackets - iterator->second.rxPackets)/(double (iterator->second.txPackets));
      double sampleThroughput = ((iterator->second.rxBytes * 8)/(double ((iterator->second.timeLastRxPacket -
          iterator->second.timeFirstRxPacket).GetSeconds())));
      double sampleDelay = (iterator->second.delaySum).GetSeconds()/(double (iterator->second.rxPackets));
      double sampleJitter = (iterator->second.jitterSum).GetSeconds()/(double (iterator->second.rxPackets - 1));
      double samplePacketSize = (iterator->second.rxBytes)/(double (iterator->second.rxPackets));
      uint32_t sampledPacketSent = iterator->second.txPackets;
      uint32_t samplePacketReceived = iterator->second.rxPackets;

      std::cout << "Throughput, delay, jitter, loss, packet size, packet sent, packet received\n";
      std::cout << "Flow " << iterator->first << " ( " << sourceAddress << ":" << sourcePort <<
          " -> " << destinationAddress << ":" << destinationPort << " )\n";
      std::cout << std::fixed
          << std::setw(6) << std::setprecision(0) << sampleThroughput << ","
          << std::setw(8) << std::setprecision(6) << sampleDelay << ","
          << std::setw(8) << std::setprecision(6) << sampleJitter << ","
          << std::setw(8) << std::setprecision(6) << sampleLoss << ","
          << std::setw(5) << std::setprecision(0) << samplePacketSize << ","
          << std::setw(8) << std::setprecision(0) << sampledPacketSent << ","
          << std::setw(8) << std::setprecision(0) << samplePacketReceived << "\n";

      std::cout.precision(6);
      std::cout << "Sender stream duration: " << senderStreamDuration << "\n";
      std::cout << "Receiver stream duration: " << receiverStreamDuration << "\n";
    }

  delete dataset;
  Simulator::Destroy();
  return 0;
}
