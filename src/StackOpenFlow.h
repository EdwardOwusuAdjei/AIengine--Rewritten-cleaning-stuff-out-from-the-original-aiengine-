/*
 * AIEngine a deep packet inspector reverse engineering engine.
 *
 * Copyright (C) 2013-2014  Luis Campo Giralte
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
 * Written by Luis Campo Giralte <luis.camp0.2009@gmail.com> 
 *
 * Configuration diagram of the stack
 *
 *                         +--------------------+
 *                         | TCPGenericProtocol |                    
 *                         +-------+------------+                    
 *                                 |                                 
 *          +--------------------+ |              +--------------------+
 *          |     SSLProtocol    | |              | UDPGenericProtocol |
 *          +--------------+-----+ |              +-----------+--------+
 *                         |       |                          |      
 * +--------------------+  |       |  +--------------------+  |      
 * |    HTTPProtocol    |  |       |  |    DNSProtocol     |  |      
 * +------------------+-+  |       |  +------------+-------+  |      
 *                    |    |       |               |          |      
 *                 +--+----+-------+----+    +-----+----------+---+  
 *                 |    TCPProtocol     |    |    UDPProtocol     |  
 *                 +------------------+-+    +-+------------------+  
 *                                    |        |                     
 *      +--------------------+        |        |                     
 *      |   ICMPProtocol     +-----+  |        |                     
 *      +--------------------+     |  |        |                     
 *                               +-+--+--------+------+              
 *                               |     IPProtocol     |              
 *                               +---------+----------+              
 *                                         |                         
 *                               +---------+----------+              
 *                               |  EthernetProtocol  |              
 *                               +---------+----------+              
 *                                         |                         
 *                               +---------+----------+              
 *                               |  OpenFlowProtocol  |              
 *                               +---------+----------+              
 *                                         |                         
 *                               +---------+----------+              
 *                               |    TCPProtocol     |              
 *                               +---------+----------+              
 *                                         |                         
 *                               +---------+----------+              
 *                         +---> |     IPProtocol     | <---+        
 *                         |     +---------+----------+     |        
 *                         |               |                |        
 *                +--------+-----------+   |   +------------+-------+
 *                |    VLANProtocol    |   |   |    MPLSProtocol    |
 *                +--------+-----------+   |   +------------+-------+
 *                         |               |                |        
 *                         |     +---------+----------+     |        
 *                         +-----+  EthernetProtocol  +-----+        
 *                               +--------------------+              
 *
 */
#ifndef SRC_STACKOPENFLOW_H_
#define SRC_STACKOPENFLOW_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <chrono>
#include <string>
#ifdef HAVE_LIBLOG4CXX
#include "log4cxx/logger.h"
#endif
#include "Multiplexer.h"
#include "FlowForwarder.h"
#include "protocols/ethernet/EthernetProtocol.h"
#include "protocols/vlan/VLanProtocol.h"
#include "protocols/mpls/MPLSProtocol.h"
#include "protocols/ip/IPProtocol.h"
#include "protocols/udp/UDPProtocol.h"
#include "protocols/tcp/TCPProtocol.h"
#include "protocols/openflow/OpenFlowProtocol.h"
#include "protocols/tcpgeneric/TCPGenericProtocol.h"
#include "protocols/udpgeneric/UDPGenericProtocol.h"
#include "protocols/icmp/ICMPProtocol.h"
#include "protocols/http/HTTPProtocol.h"
#include "protocols/ssl/SSLProtocol.h"
#include "protocols/dns/DNSProtocol.h"
#include "protocols/frequency/FrequencyProtocol.h"
#include "flow/FlowManager.h"
#include "flow/FlowCache.h"
#include "NetworkStack.h"

namespace aiengine {

class StackOpenFlow: public NetworkStack
{
public:
	explicit StackOpenFlow();
        virtual ~StackOpenFlow() {}

        const char* getName() { return name_.c_str();} 
        void setName(char *name) { name_ = name;}

        void setLinkLayerMultiplexer(MultiplexerPtrWeak mux) { }
        MultiplexerPtrWeak getLinkLayerMultiplexer() { return mux_eth_;}
	
	void showFlows(std::basic_ostream<char>& out);
	void showFlows() { showFlows(std::cout);}

	void setTotalTCPFlows(int value);
	void setTotalUDPFlows(int value) { 

		flow_cache_udp_vir_->createFlows(value);
		dns_->createDNSDomains(value / 2); 
	}

	void enableNIDSEngine(bool value);
	void enableFrequencyEngine(bool value);
	void enableLinkLayerTagging(std::string type);

	void setFlowsTimeout(int timeout);

#ifdef PYTHON_BINDING
        FlowManager &getTCPFlowManager() { return *flow_table_tcp_vir_.get();}
        FlowManager &getUDPFlowManager() { return *flow_table_udp_vir_.get();}
        
        void setTCPIPSetManager(IPSetManager& ipset_mng) { tcp_vir_->setIPSetManager(ipset_mng);}
        void setUDPIPSetManager(IPSetManager& ipset_mng) { udp_vir_->setIPSetManager(ipset_mng);}
#else
        void setTCPIPSetManager(SharedPointer<IPSetManager> ipset_mng) { tcp_vir_->setIPSetManager(ipset_mng);}
        void setUDPIPSetManager(SharedPointer<IPSetManager> ipset_mng) { udp_vir_->setIPSetManager(ipset_mng);}

        FlowManagerPtrWeak getTCPFlowManager() { return flow_table_tcp_vir_;}
        FlowManagerPtrWeak getUDPFlowManager() { return flow_table_udp_vir_;}
#endif

private:
	std::string name_;
#ifdef HAVE_LIBLOG4CXX
	static log4cxx::LoggerPtr logger;
#endif
        //Protocols
        EthernetProtocolPtr eth_,eth_vir_;
        VLanProtocolPtr vlan_;
        MPLSProtocolPtr mpls_;
        IPProtocolPtr ip_,ip_vir_;
        UDPProtocolPtr udp_vir_;
        TCPProtocolPtr tcp_,tcp_vir_;
        OpenFlowProtocolPtr of_;
        ICMPProtocolPtr icmp_;
        HTTPProtocolPtr http_;
        SSLProtocolPtr ssl_;
	DNSProtocolPtr dns_;
	TCPGenericProtocolPtr tcp_generic_;
	UDPGenericProtocolPtr udp_generic_;
	FrequencyProtocolPtr freqs_tcp_;
	FrequencyProtocolPtr freqs_udp_;
	
        // Multiplexers
        MultiplexerPtr mux_eth_,mux_eth_vir_;
        MultiplexerPtr mux_vlan_;
        MultiplexerPtr mux_mpls_;
        MultiplexerPtr mux_ip_,mux_ip_vir_;
        MultiplexerPtr mux_udp_vir_;
        MultiplexerPtr mux_of_;
        MultiplexerPtr mux_tcp_,mux_tcp_vir_;
        MultiplexerPtr mux_icmp_;

        // FlowManager and FlowCache
        FlowCachePtr flow_cache_tcp_;
        FlowCachePtr flow_cache_udp_vir_;
        FlowCachePtr flow_cache_tcp_vir_;
        FlowManagerPtr flow_table_tcp_;
        FlowManagerPtr flow_table_udp_vir_;
        FlowManagerPtr flow_table_tcp_vir_;

        // FlowForwarders
        FlowForwarderPtr ff_of_;
        FlowForwarderPtr ff_tcp_;
        FlowForwarderPtr ff_tcp_vir_;
        FlowForwarderPtr ff_udp_vir_;
        FlowForwarderPtr ff_http_;
        FlowForwarderPtr ff_ssl_;
        FlowForwarderPtr ff_dns_;
        FlowForwarderPtr ff_tcp_generic_;
        FlowForwarderPtr ff_udp_generic_;
	FlowForwarderPtr ff_tcp_freqs_;
	FlowForwarderPtr ff_udp_freqs_;

};

typedef std::shared_ptr<StackOpenFlow> StackOpenFlowPtr;

} // namespace aiengine

#endif  // SRC_STACKOPENFLOW_H_