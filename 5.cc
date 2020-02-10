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
 */

// Network topology
//
//       n0    n1   n2   n3
//       |     |    |    |
//     =====================
//
//  node n0,n1,n3 pings to node n2
//  node n0 generates protocol 2 (IGMP) to node n3

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"

#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CsmaPingExample");

/*
static void SinkRx (Ptr<const Packet> p, const Address &ad)
{
  //std::cout << *p << std::endl;
}

static void PingRtt (std::string context, Time rtt)
{
  //std::cout << context << " " << rtt << std::endl;
}
*/

int
main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);
  Time interPacketInterval = Seconds(1.);
  // Here, we will explicitly create four nodes.
  NS_LOG_UNCOND("Create nodes.");
  NodeContainer c;
  c.Create (6);

  // connect all our nodes to a shared channel.
  NS_LOG_UNCOND ("Build Topology.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  csma.SetDeviceAttribute ("EncapsulationMode", StringValue ("Llc"));
  NetDeviceContainer devs = csma.Install (c);

  // add an ip stack to all nodes.
  NS_LOG_UNCOND ("Add ip stack.");
  InternetStackHelper ipStack;
  ipStack.Install (c);

  // assign ip addresses
  NS_LOG_UNCOND ("Assign ip addresses.");
  Ipv4AddressHelper ip;
  ip.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer addresses = ip.Assign (devs);

  NS_LOG_UNCOND ("Create Source");
//  Config::SetDefault ("ns3::Ipv4RawSocketImpl::Protocol", StringValue ("2"));
  InetSocketAddress dst = InetSocketAddress (addresses.GetAddress (3));
  OnOffHelper onoff = OnOffHelper ("ns3::UdpSocketFactory", dst);
  onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("PacketSize", UintegerValue (1100));
  onoff.SetAttribute ("DataRate", StringValue ("50Mbps")); //bit/s


  ApplicationContainer apps = onoff.Install (c.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  NS_LOG_UNCOND ("Create Sink.");
  PacketSinkHelper sink = PacketSinkHelper ("ns3::UdpSocketFactory", dst);
  apps = sink.Install (c.Get (3));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (11.0));

  NS_LOG_UNCOND ("Create pinger");
  V4PingHelper ping = V4PingHelper (addresses.GetAddress (0));
  ping.SetAttribute("Interval", TimeValue(interPacketInterval));
  NodeContainer pingers;
  pingers.Add (c.Get (3));
  pingers.Add (c.Get (1));
  pingers.Add (c.Get (2));
  pingers.Add (c.Get (4));
  pingers.Add (c.Get (5));
  apps = ping.Install (pingers);
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (10.0));

/*
  NS_LOG_INFO ("Configure Tracing.");
  // first, pcap tracing in non-promiscuous mode
  csma.EnablePcapAll ("csma-ping", false);

  // then, print what the packet sink receives.
  Config::ConnectWithoutContext ("/NodeList/3/ApplicationList/0/$ns3::PacketSink/Rx", 
                                 MakeCallback (&SinkRx));
  // finally, print the ping rtts.
  Config::Connect ("/NodeList/ApplicationList/$ns3::V4Ping/Rtt",
                   MakeCallback (&PingRtt));

  Packet::EnablePrinting ();
*/

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  Simulator::Stop (Seconds(10));

  NS_LOG_UNCOND ("Run Simulation.");
  Simulator::Run ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      NS_LOG_UNCOND ("Flow ID:" << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n");
      NS_LOG_UNCOND ("Tx Packets:" << i->second.txPackets << "\n");
      NS_LOG_UNCOND ("Rx Packets:" << i->second.rxPackets << "\n");
      NS_LOG_UNCOND ("Throughput:" << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ())/ 1024  << " Kbps\n");
    }

  Simulator::Destroy ();
  NS_LOG_UNCOND ("Done.");
}
