/*
 * AIEngine a deep packet inspector reverse engineering engine.
 *
 * Copyright (C) 2013  Luis Campo Giralte
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 * Written by Luis Campo Giralte <luis.camp0.2009@gmail.com> 2013
 *
 */
#ifndef SRC_MPLS_MPLSPROTOCOL_H_
#define SRC_MPLS_MPLSPROTOCOL_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../Multiplexer.h"
#include "../FlowForwarder.h"
#include "../Protocol.h"
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

#ifndef ETHERTYPE_MPLS
#define ETHERTYPE_MPLS ETH_P_MPLS_UC 
#endif

namespace aiengine {

// A minimum MPLS Header
#define MPLS_HEADER_LEN    4

// MPLS header
// 20 bits for the label tag
// 3 bits experimental
// 1 bit for botom of label stack
// 8 bits for ttl  

class MPLSProtocol: public Protocol 
{
public:
    	explicit MPLSProtocol():Protocol("MPLSProtocol"),mpls_header_(nullptr),total_bytes_(0),
		stats_level_(0) {}
    	virtual ~MPLSProtocol() {}
	
	static const u_int16_t id = ETHERTYPE_MPLS;		// MPLS Unicast traffic	
	static const int header_size = MPLS_HEADER_LEN; 	// one header 
	int getHeaderSize() const { return header_size;}

	int64_t getTotalBytes() const { return total_bytes_;}
	int64_t getTotalPackets() const { return total_packets_;}
	int64_t getTotalValidatedPackets() const { return total_validated_packets_;}
	int64_t getTotalMalformedPackets() const { return total_malformed_packets_;}

	void processFlow(Flow *flow) {} // No flow to process
	void processPacket(Packet& packet);

	void setStatisticsLevel(int level) { stats_level_ = level;}
	void statistics(std::basic_ostream<char>& out);
	void statistics() { statistics(std::cout);}

        void setMultiplexer(MultiplexerPtrWeak mux) { mux_ = mux; }
        MultiplexerPtrWeak getMultiplexer() { return mux_;}

        void setFlowForwarder(FlowForwarderPtrWeak ff) {}
        FlowForwarderPtrWeak getFlowForwarder() { FlowForwarderPtrWeak ptr; return ptr; }

#ifdef PYTHON_BINDING
        void setDatabaseAdaptor(boost::python::object &dbptr) {} ;
#endif

        void setHeader(unsigned char *raw_packet) {
        
		mpls_header_ = raw_packet;
        }

	// Condition for say that a packet is MPLS 
	bool mplsChecker(Packet& packet) { 
	
		int length = packet.getLength();
	
		if (length >= header_size) {
			setHeader(packet.getPayload());
			++total_validated_packets_; 
			return true;
		} else {
			++total_malformed_packets_;
			return false;
		}
	}

	//unsigned char *getPayload() const { return mpls_header_;};

private:
	int stats_level_;
	MultiplexerPtrWeak mux_;
	unsigned char *mpls_header_;
	int64_t total_bytes_;
};

typedef std::shared_ptr<MPLSProtocol> MPLSProtocolPtr;

} // namespace aiengine

#endif  // SRC_MPLS_MPLSPROTOCOL_H_
