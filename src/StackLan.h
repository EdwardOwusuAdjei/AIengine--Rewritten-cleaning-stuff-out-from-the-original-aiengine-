#ifndef _StackLan_H_
#define _StackLan_H_

#include <string>
#include "Multiplexer.h"
#include "./ethernet/EthernetProtocol.h"
#include "./ip/IPProtocol.h"
#include "./udp/UDPProtocol.h"
#include "./tcp/TCPProtocol.h"
#include "./icmp/ICMPProtocol.h"
#include "./flow/FlowManager.h"
#include "./flow/FlowCache.h"

struct StackLan
{
        EthernetProtocolPtr eth;
        IPProtocolPtr ip;
        UDPProtocolPtr udp;
        TCPProtocolPtr tcp;
        ICMPProtocolPtr icmp;
        MultiplexerPtr mux_eth;
        MultiplexerPtr mux_ip;
        MultiplexerPtr mux_udp;
        MultiplexerPtr mux_tcp;
        MultiplexerPtr mux_icmp;
	FlowManagerPtr flow_table_udp;
	FlowManagerPtr flow_table_tcp;
	FlowCachePtr flow_cache_udp;
	FlowCachePtr flow_cache_tcp;

        StackLan()
        {
		// Allocate all the objects
                tcp = TCPProtocolPtr(new TCPProtocol());
                udp = UDPProtocolPtr(new UDPProtocol());
                ip = IPProtocolPtr(new IPProtocol());
                eth = EthernetProtocolPtr(new EthernetProtocol());
		icmp = ICMPProtocolPtr(new ICMPProtocol());
                mux_eth = MultiplexerPtr(new Multiplexer());
                mux_ip = MultiplexerPtr(new Multiplexer());
                mux_udp = MultiplexerPtr(new Multiplexer());
                mux_tcp = MultiplexerPtr(new Multiplexer());
                mux_icmp = MultiplexerPtr(new Multiplexer());
		flow_table_udp = FlowManagerPtr(new FlowManager());
		flow_table_tcp = FlowManagerPtr(new FlowManager());
		flow_cache_udp = FlowCachePtr(new FlowCache());
		flow_cache_tcp = FlowCachePtr(new FlowCache());

                //configure the eth
                eth->setMultiplexer(mux_eth);
                mux_eth->setProtocol(static_cast<ProtocolPtr>(eth));
		mux_eth->setProtocolIdentifier(0);
                mux_eth->setHeaderSize(eth->getHeaderSize());
                mux_eth->addChecker(std::bind(&EthernetProtocol::ethernetChecker,eth));

                // configure the ip
                ip->setMultiplexer(mux_ip);
                mux_ip->setProtocol(static_cast<ProtocolPtr>(ip));
		mux_ip->setProtocolIdentifier(ETHERTYPE_IP);
                mux_ip->setHeaderSize(ip->getHeaderSize());
                mux_ip->addChecker(std::bind(&IPProtocol::ipChecker,ip));
                mux_ip->addPacketFunction(std::bind(&IPProtocol::processPacket,ip));

                //configure the icmp
                icmp->setMultiplexer(mux_icmp);
                mux_icmp->setProtocol(static_cast<ProtocolPtr>(icmp));
                mux_icmp->setProtocolIdentifier(IPPROTO_ICMP);
                mux_icmp->setHeaderSize(icmp->getHeaderSize());
                mux_icmp->addChecker(std::bind(&ICMPProtocol::icmpChecker,icmp));

                //configure the udp
                udp->setMultiplexer(mux_udp);
                mux_udp->setProtocol(static_cast<ProtocolPtr>(udp));
		mux_udp->setProtocolIdentifier(IPPROTO_UDP);
                mux_udp->setHeaderSize(udp->getHeaderSize());
                mux_udp->addChecker(std::bind(&UDPProtocol::udpChecker,udp));
                mux_udp->addPacketFunction(std::bind(&UDPProtocol::processPacket,udp));

                //configure the tcp 
                tcp->setMultiplexer(mux_tcp);
                mux_tcp->setProtocol(static_cast<ProtocolPtr>(tcp));
		mux_tcp->setProtocolIdentifier(IPPROTO_TCP);
                mux_tcp->setHeaderSize(tcp->getHeaderSize());
                mux_tcp->addChecker(std::bind(&TCPProtocol::tcpChecker,tcp));
                mux_tcp->addPacketFunction(std::bind(&TCPProtocol::processPacket,tcp));

		// configure the multiplexers
                mux_eth->addUpMultiplexer(mux_ip,ETHERTYPE_IP);
                mux_ip->addDownMultiplexer(mux_eth);
                mux_ip->addUpMultiplexer(mux_udp,IPPROTO_UDP);
                mux_udp->addDownMultiplexer(mux_ip);
                mux_ip->addUpMultiplexer(mux_tcp,IPPROTO_TCP);
                mux_tcp->addDownMultiplexer(mux_ip);
                mux_ip->addUpMultiplexer(mux_icmp,IPPROTO_ICMP);
                mux_icmp->addDownMultiplexer(mux_ip);
		
		// Connect the FlowManager and FlowCache
		flow_cache_udp->createFlows(1024*16);
		flow_cache_tcp->createFlows(1024*32);
		
		tcp->setFlowCache(flow_cache_tcp);
		tcp->setFlowManager(flow_table_tcp);
				
		udp->setFlowCache(flow_cache_udp);
		udp->setFlowManager(flow_table_udp);
        }

	void statistics()
	{
		eth->statistics();
		mux_eth->statistics();
		std::cout << std::endl;
		ip->statistics();
		mux_ip->statistics();
		std::cout << std::endl;
		tcp->statistics();
		mux_tcp->statistics();
		std::cout << std::endl;
		udp->statistics();
		mux_udp->statistics();
		std::cout << std::endl;
		icmp->statistics();
		mux_icmp->statistics();
	}

        ~StackLan() {
        }
};


#endif