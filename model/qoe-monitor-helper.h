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

#ifndef QOEMONITORHELPER_H_
#define QOEMONITORHELPER_H_

namespace ns3 {

class QoEMonitorHelper
{
protected:
  // TODO: define the class' attributes

public:
  QoEMonitorHelper ();
  virtual ~QoEMonitorHelper ();

  void ReadRawFile (char* filePath);
  void SetCodec (Codec* codec);
  void EncodeFile ();
  void EncodeFile (char* filePath, Codec* codec);
  void DecodeFile ();
  void DecodeFile (Codec* codec);
  void SetMetric (Metric* metric);
  void SetTransmissionParameters (int mtu);
  void SetErrorCorrection (ErrorCorrectionStrategy* errorCorrectionStrategy);
  void SetMaxJitter (int maxJitter);
  void SetSender (Ptr<Node> sender);
  void SetSenders (Ptr<NodeContainer> senders);
  void SetReceiver (Ptr<Node> receiver);
  void SetReceivers (Ptr<NodeContainer> receivers);
  void SetInitialAdjacencyMatrix (Vector<Vector<bool> > adjacencyMatrix);
  void Install ();
  void ReconstructFiles ();
  void FixFiles ();
  void EvaluateQoe ();
  void ExportResults (char* filePath);
};

} // namespace ns3

#endif /* QOEMONITORHELPER_H_ */
