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

#ifndef PACKET_TRACE_STRUCTURE_H_
#define PACKET_TRACE_STRUCTURE_H_

/* Declaration of the row structure regarding the packet trace */
typedef struct
{
  unsigned int m_packetId;
  unsigned int m_packetSize;
  double m_playbackTimestamp;
  double m_decodingTimestamp;
  unsigned long int m_rtpTimestamp;

  /* This stores the number of fragments composing a single multimedia-unit whose
   * this packet belongs to. */
  unsigned int m_numberOfFragments;
} PacketTraceRow;

/* Declaration of the row structure regarding the sender trace */
typedef struct
{
  unsigned int m_packetId;
  double m_senderTimestamp;
} SenderTraceRow;

/* Declaration of the row structure regarding the receiver trace */
typedef struct
{
  unsigned int m_packetId;
  double m_receiverTimestamp;
} ReceiverTraceRow;

/* Declaration of the row structure regarding the jitter trace */
typedef struct
{
  unsigned int m_packetId;
  double m_receptionTime;
  double m_jitter;
} JitterTraceRow;

#endif /* PACKET_TRACE_STRUCTURE_H_ */
