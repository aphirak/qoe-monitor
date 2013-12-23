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

#ifndef ERRORCORRECTIONSTRATEGY_H_
#define ERRORCORRECTIONSTRATEGY_H_

namespace ns3 {

class ErrorCorrectionStrategy
{
protected:
  // TODO: class' attributes
public:
  ErrorCorrectionStrategy ();
  virtual ~ErrorCorrectionStrategy ();

  virtual void FixMultimediaFile (Ptr<SimulationDataset> simulationDataset) = 0;
};

} // namespace ns3

#endif /* ERRORCORRECTIONSTRATEGY_H_ */
