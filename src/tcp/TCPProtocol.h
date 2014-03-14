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
#ifndef SRC_TCP_TCPPROTOCOL_H_
#define SRC_TCP_TCPPROTOCOL_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../Multiplexer.h"
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../Protocol.h"
#include "../flow/FlowManager.h"
#include "../flow/FlowCache.h"
#include "../FlowForwarder.h"
#include "../Cache.h"
#include "TCPStates.h"
#include "TCPInfo.h"

namespace aiengine {

class TCPProtocol: public Protocol 
{
public:
    	explicit TCPProtocol():tcp_header_(nullptr),current_flow_(nullptr),total_bytes_(0),
		stats_level_(0),
#ifdef PYTHON_BINDING
		is_set_db_(false),
#endif
		tcp_info_cache_(new Cache<TCPInfo>("TCP info cache")) { name_="TCPProtocol";}
    	explicit TCPProtocol(std::string name):tcp_header_(nullptr),current_flow_(nullptr),total_bytes_(0),
		stats_level_(0),
#ifdef PYTHON_BINDING
                is_set_db_(false),
#endif
		tcp_info_cache_(new Cache<TCPInfo>("TCP info cache")) { name_ = name;}
    	virtual ~TCPProtocol() {}

	static const u_int16_t id = IPPROTO_TCP;
	static const int header_size = 20;
	int getHeaderSize() const { return header_size;}

	int64_t getTotalBytes()  const { return total_bytes_; }
	int64_t getTotalPackets() const { return total_packets_;}
	int64_t getTotalValidatedPackets() const { return total_validated_packets_;}
	int64_t getTotalMalformedPackets() const { return total_malformed_packets_;}

        void setFlowForwarder(FlowForwarderPtrWeak ff) { flow_forwarder_= ff; }
        FlowForwarderPtrWeak getFlowForwarder() { return flow_forwarder_;}

        void setMultiplexer(MultiplexerPtrWeak mux) { mux_ = mux; }
        MultiplexerPtrWeak getMultiplexer() { return mux_;}

#ifdef PYTHON_BINDING
	void setDatabaseAdaptor(boost::python::object &dbptr);
#endif

        const char *getName() { return name_.c_str();}

	void processFlow(Flow *flow) {}; // This protocol generates flows but not for destination.
	void processPacket(Packet &packet);
	void computeState(Flow *flow,int32_t bytes);

	void setStatisticsLevel(int level) { stats_level_ = level;}
	void statistics(std::basic_ostream<char>& out);
	void statistics() { statistics(std::cout);}

        void setHeader(unsigned char *raw_packet) {
        
                tcp_header_ = reinterpret_cast <struct tcphdr*> (raw_packet);
        }

	// Condition for say that a packet is tcp 
	bool tcpChecker(Packet &packet) { 
	
                int length = packet.getLength();

		if (length >= header_size) {
                	setHeader(packet.getPayload());
			++total_validated_packets_;
			total_bytes_ += length; 
			return true;
		} else {
			++total_malformed_packets_;
			return false;
		}
	}


#ifdef __FREEBSD__
    	u_int16_t getSrcPort() const { return ntohs(tcp_header_->th_sport); }
    	u_int16_t getDstPort() const { return ntohs(tcp_header_->th_dport); }
    	u_int32_t getSequence() const  { return ntohl(tcp_header_->th_seq); }
    	u_int32_t getAckSequence() const  { return ntohl(tcp_header_->th_ack); }
    	// u_int16_t getWindow() const { return tcp_header_->window; }
    	bool isSyn() const { return (tcp_header_->th_flags & TH_SYN) == TH_SYN; }
    	bool isFin() const { return (tcp_header_->th_flags & TH_FIN) == TH_FIN; }
    	bool isAck() const { return (tcp_header_->th_flags & TH_ACK) == TH_ACK; }
    	bool isRst() const { return (tcp_header_->th_flags & TH_RST) == TH_RST; }
    	bool isPushSet() const { return (tcp_header_->th_flags & TH_PUSH) == TH_PUSH; }
    	// unsigned int getTcpSegmentLength() const { return ntohs(ip->tot_len) - ip->ihl * 4; }
    	// unsigned int getPayloadLength() const { return ntohs(ip->tot_len) - 20 /* ip->ihl * 4 */ - tcp->doff * 4; }
    	unsigned int getTcpHdrLength() const { return tcp_header_->th_off * 4; }
    	unsigned char* getPayload() const { return (unsigned char*)tcp_header_ +getTcpHdrLength(); }
#else
    	bool isSyn() const { return tcp_header_->syn == 1; }
    	bool isFin() const { return tcp_header_->fin == 1; }
    	bool isAck() const { return tcp_header_->ack == 1; }
    	bool isRst() const { return tcp_header_->rst == 1; }
    	bool isPushSet() const { return tcp_header_->psh == 1; }
    	uint32_t getSequence() const  { return ntohl(tcp_header_->seq); }
    	uint32_t getAckSequence() const  { return ntohl(tcp_header_->ack_seq); }
    	u_int16_t getSrcPort() const { return ntohs(tcp_header_->source); }
    	u_int16_t getDstPort() const { return ntohs(tcp_header_->dest); }
    	unsigned int getTcpHdrLength() const { return tcp_header_->doff * 4; }
    	unsigned char* getPayload() const { return (unsigned char*)tcp_header_ +getTcpHdrLength(); }
#endif
        void setFlowManager(FlowManagerPtr flow_mng) { flow_table_ = flow_mng;}
        FlowManagerPtr getFlowManager() { return flow_table_; }

        void setFlowCache(FlowCachePtr flow_cache) { flow_cache_ = flow_cache; } 
        FlowCachePtr getFlowCache() { return flow_cache_;}

        void createTCPInfo(int number) { tcp_info_cache_->create(number);}
        void destroyTCPInfo(int number) { tcp_info_cache_->destroy(number);}

	Flow *getCurrentFlow() { return current_flow_;} // used just for testing pourposes
private:
        SharedPointer<Flow> getFlow();

	int stats_level_;
	MultiplexerPtrWeak mux_;
	FlowForwarderPtrWeak flow_forwarder_;
	FlowManagerPtr flow_table_;
	FlowCachePtr flow_cache_;
	Cache<TCPInfo>::CachePtr tcp_info_cache_;
	struct tcphdr *tcp_header_;
	Flow *current_flow_;
	int64_t total_bytes_;
#ifdef PYTHON_BINDING
	boost::python::object dbptr_;
	bool is_set_db_;
#endif
};

typedef std::shared_ptr<TCPProtocol> TCPProtocolPtr;

} // namespace aiengine

#endif  // SRC_TCP_TCPPROTOCOL_H_
