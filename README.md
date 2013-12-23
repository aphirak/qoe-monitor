# This is my README
This is the module for using in ns3 to evaluate QoE

According to the following paper:

D. Saladino, A. Paganelli, M. Casoni, A tool for multimedia quality assessment in NS3: QoE Monitor, Simulation Modelling Practice and Theory, Volume 32, March 2013, Pages 30-41, ISSN 1569-190X, http://dx.doi.org/10.1016/j.simpat.2012.11.011.
(http://www.sciencedirect.com/science/article/pii/S1569190X12001669)
Keywords: QoE evaluation; NS-3; PSNR; SSIM

It has been download from 
http://sourceforge.net/projects/ns3qoemonitor/


Description

QoE Monitor is an open-source software module for Network Simulator 3 (NS-3), usable to perform Quality-of-Experience (QoE) evaluations of multimedia communications in simulative networks. More in detail, it is able to assess the user perceived quality of audio and/or video multimedia streams. Its design has been inspired by EvalVid (link to http://www.tkn.tu-berlin.de/menue/research/evalvid/).

QoE Monitor is currently developed by the Networking Research group of University of Modena and Reggio Emilia (http://www.unimore.it/).

List of currently available features:
- PSNR/SSIM computation for H.264 video transmission
- G.711 audio transmission
- Basic RTP emulation
- Receiver side jitter estimation, according to RFC 3550

To do:
- support new audio and video codecs
- support new audio and video formats
- develop objective audio evaluation metric(s)

Update 02/27/2013:
The latest archive has been updated to include some new features.
