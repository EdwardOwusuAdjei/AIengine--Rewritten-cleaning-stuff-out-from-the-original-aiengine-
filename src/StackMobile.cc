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
#include "StackMobile.h"

namespace aiengine {

#ifdef HAVE_LIBLOG4CXX
log4cxx::LoggerPtr StackMobile::logger(log4cxx::Logger::getLogger("aiengine.stackmobile"));
#endif

StackMobile::StackMobile() {

	name_ = "Mobile Network Stack";

	// Allocate all the Protocol objects
        eth_= EthernetProtocolPtr(new EthernetProtocol());
	addProtocol(eth_);
        vlan_= VLanProtocolPtr(new VLanProtocol());
	addProtocol(vlan_);
        mpls_= MPLSProtocolPtr(new MPLSProtocol());
	addProtocol(mpls_);
        ip_low_ = IPProtocolPtr(new IPProtocol("IPProtocol low"));
	addProtocol(ip_low_);
        udp_low_ = UDPProtocolPtr(new UDPProtocol("UDPProtocol low"));
	addProtocol(udp_low_);
        gprs_ = GPRSProtocolPtr(new GPRSProtocol());
	addProtocol(gprs_);

	ip_high_ = IPProtocolPtr(new IPProtocol());
	addProtocol(ip_high_);

	udp_high_ = UDPProtocolPtr(new UDPProtocol());
	addProtocol(udp_high_);
        tcp_ = TCPProtocolPtr(new TCPProtocol());
	addProtocol(tcp_);
        icmp_ = ICMPProtocolPtr(new ICMPProtocol());
	addProtocol(icmp_);
        http_ = HTTPProtocolPtr(new HTTPProtocol());
	addProtocol(http_);
        ssl_ = SSLProtocolPtr(new SSLProtocol());
	addProtocol(ssl_);
        dns_ = DNSProtocolPtr(new DNSProtocol());
	addProtocol(dns_);
        tcp_generic_ = TCPGenericProtocolPtr(new TCPGenericProtocol());
	addProtocol(tcp_generic_);
        udp_generic_ = UDPGenericProtocolPtr(new UDPGenericProtocol());
	addProtocol(udp_generic_);
	freqs_tcp_ = FrequencyProtocolPtr(new FrequencyProtocol("TCPFrequencyProtocol"));
	addProtocol(freqs_tcp_);
	freqs_udp_ = FrequencyProtocolPtr(new FrequencyProtocol("UDPFrequencyProtocol"));
	addProtocol(freqs_udp_);

	// Allocate the Multiplexers
	mux_eth_ = MultiplexerPtr(new Multiplexer());
	mux_vlan_ = MultiplexerPtr(new Multiplexer());
	mux_mpls_ = MultiplexerPtr(new Multiplexer());
	mux_ip_low_ = MultiplexerPtr(new Multiplexer());
	mux_ip_high_ = MultiplexerPtr(new Multiplexer());
	mux_udp_low_ = MultiplexerPtr(new Multiplexer());
	mux_udp_high_ = MultiplexerPtr(new Multiplexer());
	mux_tcp_ = MultiplexerPtr(new Multiplexer());
	mux_icmp_ = MultiplexerPtr(new Multiplexer());
	mux_gprs_ = MultiplexerPtr(new Multiplexer());

	// Allocate the flow caches and tables
       	flow_cache_tcp_ = FlowCachePtr(new FlowCache());
        flow_cache_udp_low_ = FlowCachePtr(new FlowCache());
        flow_cache_udp_high_ = FlowCachePtr(new FlowCache());
        flow_table_tcp_ = FlowManagerPtr(new FlowManager());
        flow_table_udp_high_ = FlowManagerPtr(new FlowManager());
        flow_table_udp_low_ = FlowManagerPtr(new FlowManager());

        // Link the FlowCaches to their corresponding FlowManager for timeouts
        flow_table_udp_low_->setFlowCache(flow_cache_udp_low_);
        flow_table_udp_high_->setFlowCache(flow_cache_udp_high_);
        flow_table_tcp_->setFlowCache(flow_cache_tcp_);

	ff_tcp_ = FlowForwarderPtr(new FlowForwarder());
	ff_udp_low_ = FlowForwarderPtr(new FlowForwarder());
	ff_udp_high_ = FlowForwarderPtr(new FlowForwarder());
	ff_http_ = FlowForwarderPtr(new FlowForwarder());
	ff_ssl_ = FlowForwarderPtr(new FlowForwarder());
	ff_dns_ = FlowForwarderPtr(new FlowForwarder());
	ff_gprs_ = FlowForwarderPtr(new FlowForwarder());
        ff_tcp_generic_ = FlowForwarderPtr(new FlowForwarder());
        ff_udp_generic_ = FlowForwarderPtr(new FlowForwarder());
        ff_tcp_freqs_ = FlowForwarderPtr(new FlowForwarder());
        ff_udp_freqs_ = FlowForwarderPtr(new FlowForwarder());

	//configure the Ethernet Layer 
	eth_->setMultiplexer(mux_eth_);
	mux_eth_->setProtocol(static_cast<ProtocolPtr>(eth_));
	mux_eth_->setProtocolIdentifier(0);
	mux_eth_->setHeaderSize(eth_->getHeaderSize());
	mux_eth_->addChecker(std::bind(&EthernetProtocol::ethernetChecker,eth_,std::placeholders::_1));

        //configure the VLan tagging Layer
        vlan_->setMultiplexer(mux_vlan_);
        mux_vlan_->setProtocol(static_cast<ProtocolPtr>(vlan_));
        mux_vlan_->setProtocolIdentifier(ETHERTYPE_VLAN);
        mux_vlan_->setHeaderSize(vlan_->getHeaderSize());
        mux_vlan_->addChecker(std::bind(&VLanProtocol::vlanChecker,vlan_,std::placeholders::_1));
	mux_vlan_->addPacketFunction(std::bind(&VLanProtocol::processPacket,vlan_,std::placeholders::_1));

        //configure the MPLS Layer
        mpls_->setMultiplexer(mux_mpls_);
        mux_mpls_->setProtocol(static_cast<ProtocolPtr>(mpls_));
        mux_mpls_->setProtocolIdentifier(ETHERTYPE_MPLS);
        mux_mpls_->setHeaderSize(mpls_->getHeaderSize());
        mux_mpls_->addChecker(std::bind(&MPLSProtocol::mplsChecker,mpls_,std::placeholders::_1));
	mux_mpls_->addPacketFunction(std::bind(&MPLSProtocol::processPacket,mpls_,std::placeholders::_1));

	// configure the low IP Layer 
	ip_low_->setMultiplexer(mux_ip_low_);
	mux_ip_low_->setProtocol(static_cast<ProtocolPtr>(ip_low_));
	mux_ip_low_->setProtocolIdentifier(ETHERTYPE_IP);
	mux_ip_low_->setHeaderSize(ip_low_->getHeaderSize());
	mux_ip_low_->addChecker(std::bind(&IPProtocol::ipChecker,ip_low_,std::placeholders::_1));
	mux_ip_low_->addPacketFunction(std::bind(&IPProtocol::processPacket,ip_low_,std::placeholders::_1));

	//configure the low UDP Layer 
	udp_low_->setMultiplexer(mux_udp_low_);
	mux_udp_low_->setProtocol(static_cast<ProtocolPtr>(udp_low_));
	ff_udp_low_->setProtocol(static_cast<ProtocolPtr>(udp_low_));
	mux_udp_low_->setProtocolIdentifier(IPPROTO_UDP);
	mux_udp_low_->setHeaderSize(udp_low_->getHeaderSize());
	mux_udp_low_->addChecker(std::bind(&UDPProtocol::udpChecker,udp_low_,std::placeholders::_1));
	mux_udp_low_->addPacketFunction(std::bind(&UDPProtocol::processPacket,udp_low_,std::placeholders::_1));

	//configure the gprs
	gprs_->setFlowForwarder(ff_gprs_);
	gprs_->setMultiplexer(mux_gprs_);
	mux_gprs_->setProtocol(static_cast<ProtocolPtr>(gprs_));
	mux_gprs_->setHeaderSize(gprs_->getHeaderSize());
	mux_gprs_->setProtocolIdentifier(0);
	ff_gprs_->setProtocol(static_cast<ProtocolPtr>(gprs_));
	ff_gprs_->addChecker(std::bind(&GPRSProtocol::gprsChecker,gprs_,std::placeholders::_1));
	ff_gprs_->addFlowFunction(std::bind(&GPRSProtocol::processFlow,gprs_,std::placeholders::_1));

     	// configure the high ip handler
        ip_high_->setMultiplexer(mux_ip_high_);
        mux_ip_high_->setProtocol(static_cast<ProtocolPtr>(ip_high_));
        mux_ip_high_->setProtocolIdentifier(ETHERTYPE_IP);
        mux_ip_high_->setHeaderSize(ip_high_->getHeaderSize());
        mux_ip_high_->addChecker(std::bind(&IPProtocol::ipChecker,ip_high_,std::placeholders::_1));
        mux_ip_high_->addPacketFunction(std::bind(&IPProtocol::processPacket,ip_high_,std::placeholders::_1));

        // Create the HIGH UDP layer
        udp_high_->setMultiplexer(mux_udp_high_);
        mux_udp_high_->setProtocol(static_cast<ProtocolPtr>(udp_high_));
        ff_udp_high_->setProtocol(static_cast<ProtocolPtr>(udp_high_));
        mux_udp_high_->setProtocolIdentifier(IPPROTO_UDP);
        mux_udp_high_->setHeaderSize(udp_high_->getHeaderSize());
        mux_udp_high_->addChecker(std::bind(&UDPProtocol::udpChecker,udp_high_,std::placeholders::_1));
        mux_udp_high_->addPacketFunction(std::bind(&UDPProtocol::processPacket,udp_high_,std::placeholders::_1));

	//configure the TCP Layer
	tcp_->setMultiplexer(mux_tcp_);
	mux_tcp_->setProtocol(static_cast<ProtocolPtr>(tcp_));
	ff_tcp_->setProtocol(static_cast<ProtocolPtr>(tcp_));
	mux_tcp_->setProtocolIdentifier(IPPROTO_TCP);
	mux_tcp_->setHeaderSize(tcp_->getHeaderSize());
	mux_tcp_->addChecker(std::bind(&TCPProtocol::tcpChecker,tcp_,std::placeholders::_1));
	mux_tcp_->addPacketFunction(std::bind(&TCPProtocol::processPacket,tcp_,std::placeholders::_1));

        //configure the ICMP Layer
        icmp_->setMultiplexer(mux_icmp_);
        mux_icmp_->setProtocol(static_cast<ProtocolPtr>(icmp_));
        mux_icmp_->setProtocolIdentifier(IPPROTO_ICMP);
        mux_icmp_->setHeaderSize(icmp_->getHeaderSize());
        mux_icmp_->addChecker(std::bind(&ICMPProtocol::icmpChecker,icmp_,std::placeholders::_1));
	mux_icmp_->addPacketFunction(std::bind(&ICMPProtocol::processPacket,icmp_,std::placeholders::_1));

	// configure the HTTP Layer 
	http_->setFlowForwarder(ff_http_);
	ff_http_->setProtocol(static_cast<ProtocolPtr>(http_));
	ff_http_->addChecker(std::bind(&HTTPProtocol::httpChecker,http_,std::placeholders::_1));
	ff_http_->addFlowFunction(std::bind(&HTTPProtocol::processFlow,http_,std::placeholders::_1));
	
	// configure the SSL Layer 
	ssl_->setFlowForwarder(ff_ssl_);
	ff_ssl_->setProtocol(static_cast<ProtocolPtr>(ssl_));
	ff_ssl_->addChecker(std::bind(&SSLProtocol::sslChecker,ssl_,std::placeholders::_1));
	ff_ssl_->addFlowFunction(std::bind(&SSLProtocol::processFlow,ssl_,std::placeholders::_1));

        // configure the DNS Layer
        dns_->setFlowForwarder(ff_dns_);
        ff_dns_->setProtocol(static_cast<ProtocolPtr>(dns_));
        ff_dns_->addChecker(std::bind(&DNSProtocol::dnsChecker,dns_,std::placeholders::_1));
        ff_dns_->addFlowFunction(std::bind(&DNSProtocol::processFlow,dns_,std::placeholders::_1));

        // configure the TCP generic Layer
        tcp_generic_->setFlowForwarder(ff_tcp_generic_);
        ff_tcp_generic_->setProtocol(static_cast<ProtocolPtr>(tcp_generic_));
        ff_tcp_generic_->addChecker(std::bind(&TCPGenericProtocol::tcpGenericChecker,tcp_generic_,std::placeholders::_1));
        ff_tcp_generic_->addFlowFunction(std::bind(&TCPGenericProtocol::processFlow,tcp_generic_,std::placeholders::_1));

        // configure the UDP generic Layer
        udp_generic_->setFlowForwarder(ff_udp_generic_);
        ff_udp_generic_->setProtocol(static_cast<ProtocolPtr>(udp_generic_));
        ff_udp_generic_->addChecker(std::bind(&UDPGenericProtocol::udpGenericChecker,udp_generic_,std::placeholders::_1));
        ff_udp_generic_->addFlowFunction(std::bind(&UDPGenericProtocol::processFlow,udp_generic_,std::placeholders::_1));

        // configure the TCP frequencies
        freqs_tcp_->setFlowForwarder(ff_tcp_freqs_);
        ff_tcp_freqs_->setProtocol(static_cast<ProtocolPtr>(freqs_tcp_));
        ff_tcp_freqs_->addChecker(std::bind(&FrequencyProtocol::freqChecker,freqs_tcp_,std::placeholders::_1));
        ff_tcp_freqs_->addFlowFunction(std::bind(&FrequencyProtocol::processFlow,freqs_tcp_,std::placeholders::_1));

        // configure the UDP frequencies
        freqs_udp_->setFlowForwarder(ff_udp_freqs_);
        ff_udp_freqs_->setProtocol(static_cast<ProtocolPtr>(freqs_udp_));
        ff_udp_freqs_->addChecker(std::bind(&FrequencyProtocol::freqChecker,freqs_udp_,std::placeholders::_1));
        ff_udp_freqs_->addFlowFunction(std::bind(&FrequencyProtocol::processFlow,freqs_udp_,std::placeholders::_1));

	// configure the multiplexers
	mux_eth_->addUpMultiplexer(mux_ip_low_,ETHERTYPE_IP);
	mux_ip_low_->addDownMultiplexer(mux_eth_);
	mux_ip_low_->addUpMultiplexer(mux_udp_low_,IPPROTO_UDP);
	mux_udp_low_->addDownMultiplexer(mux_ip_low_);

	// configure the multiplexers of the second part
	mux_gprs_->addUpMultiplexer(mux_ip_high_,ETHERTYPE_IP);
        mux_ip_high_->addDownMultiplexer(mux_gprs_);
        mux_ip_high_->addUpMultiplexer(mux_icmp_,IPPROTO_ICMP);
	mux_icmp_->addDownMultiplexer(mux_ip_high_);
	mux_ip_high_->addUpMultiplexer(mux_tcp_,IPPROTO_TCP);
	mux_tcp_->addDownMultiplexer(mux_ip_high_);
	mux_ip_high_->addUpMultiplexer(mux_udp_high_,IPPROTO_UDP);
	mux_udp_high_->addDownMultiplexer(mux_ip_high_);

	// Connect the FlowManager and FlowCache
	tcp_->setFlowCache(flow_cache_tcp_);
	tcp_->setFlowManager(flow_table_tcp_);
			
	udp_low_->setFlowCache(flow_cache_udp_low_);
	udp_low_->setFlowManager(flow_table_udp_low_);
	
	udp_high_->setFlowCache(flow_cache_udp_high_);
	udp_high_->setFlowManager(flow_table_udp_high_);

	// The low FlowManager have a 24 hours timeout to keep the Context on memory
        flow_table_udp_low_->setTimeout(86400);

	// Configure the FlowForwarders
	udp_low_->setFlowForwarder(ff_udp_low_);
	ff_udp_low_->addUpFlowForwarder(ff_gprs_);

	tcp_->setFlowForwarder(ff_tcp_);	
	udp_high_->setFlowForwarder(ff_udp_high_);	
	
	ff_tcp_->addUpFlowForwarder(ff_http_);
	ff_tcp_->addUpFlowForwarder(ff_ssl_);
	ff_tcp_->addUpFlowForwarder(ff_tcp_generic_);
	ff_udp_high_->addUpFlowForwarder(ff_dns_);
	ff_udp_high_->addUpFlowForwarder(ff_udp_generic_);

#ifdef HAVE_LIBLOG4CXX
	LOG4CXX_INFO (logger, name_<< " ready.");
#else
        std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now();
        std::time_t now = std::chrono::system_clock::to_time_t(time_point);
#ifdef __clang__
        std::cout << "[" << std::put_time(std::localtime(&now), "%D %X") << "] ";
#else
        char mbstr[100];
        std::strftime(mbstr, 100, "%D %X", std::localtime(&now));
        std::cout << "[" << mbstr << "] ";
#endif
        std::cout << name_ << " ready." << std::endl; 
#endif
}

void StackMobile::showFlows(std::basic_ostream<char>& out) {

	out << "Flows on memory" << std::endl;
	flow_table_udp_low_->showFlows(out);
	flow_table_tcp_->showFlows(out);
	flow_table_udp_high_->showFlows(out);
}

void StackMobile::setTotalTCPFlows(int value) {

        flow_cache_tcp_->createFlows(value);
	tcp_->createTCPInfo(value);
        
	// The vast majority of the traffic of internet is HTTP
        // so create 75% of the value received for the http caches
	http_->createHTTPUris(value * 1.5);
        http_->createHTTPHosts(value * 0.75);
        http_->createHTTPUserAgents(value * 0.75);

        // The 40% of the traffic is SSL
        ssl_->createSSLHosts(value * 0.4);
}

void StackMobile::enableFrequencyEngine(bool enable) {

        int tcp_flows_created = flow_cache_tcp_->getTotalFlows();
        int udp_flows_created = flow_cache_udp_high_->getTotalFlows();

        ff_udp_high_->removeUpFlowForwarder();
        ff_tcp_->removeUpFlowForwarder();
        if (enable) {
#ifdef HAVE_LIBLOG4CXX
		LOG4CXX_INFO (logger, "Enable FrequencyEngine on " << name_ );
#else
        	std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now();
        	std::time_t now = std::chrono::system_clock::to_time_t(time_point);
#ifdef __clang__
        	std::cout << "[" << std::put_time(std::localtime(&now), "%D %X") << "] ";
#else
        	char mbstr[100];
        	std::strftime(mbstr, 100, "%D %X", std::localtime(&now));
        	std::cout << "[" << mbstr << "] ";
#endif
		std::cout <<  "Enable FrequencyEngine on " << name_ << std::endl;
#endif 
                freqs_tcp_->createFrequencies(tcp_flows_created);
                freqs_udp_->createFrequencies(udp_flows_created);

                ff_tcp_->insertUpFlowForwarder(ff_tcp_freqs_);
                ff_udp_high_->insertUpFlowForwarder(ff_udp_freqs_);
        } else {
                freqs_tcp_->destroyFrequencies(tcp_flows_created);
                freqs_udp_->destroyFrequencies(udp_flows_created);

                ff_tcp_->removeUpFlowForwarder(ff_tcp_freqs_);
                ff_udp_high_->removeUpFlowForwarder(ff_udp_freqs_);
        }
}

void StackMobile::enableNIDSEngine(bool enable) {

        if (enable) {
                ff_tcp_->removeUpFlowForwarder(ff_http_);
                ff_tcp_->removeUpFlowForwarder(ff_ssl_);
                ff_udp_high_->removeUpFlowForwarder(ff_dns_);
#ifdef HAVE_LIBLOG4CXX
                LOG4CXX_INFO (logger, "Enable NIDSEngine on " << name_ );
#else
        	std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now();
        	std::time_t now = std::chrono::system_clock::to_time_t(time_point);
#ifdef __clang__
        	std::cout << "[" << std::put_time(std::localtime(&now), "%D %X") << "] ";
#else
        	char mbstr[100];
        	std::strftime(mbstr, 100, "%D %X", std::localtime(&now));
        	std::cout << "[" << mbstr << "] ";
#endif
                std::cout << "Enable NIDSEngine on " << name_ << std::endl;
#endif
        } else {
                ff_tcp_->removeUpFlowForwarder(ff_tcp_generic_);
                ff_udp_high_->removeUpFlowForwarder(ff_udp_generic_);

                ff_tcp_->addUpFlowForwarder(ff_http_);
                ff_tcp_->addUpFlowForwarder(ff_ssl_);
                ff_tcp_->addUpFlowForwarder(ff_tcp_generic_);
                ff_udp_high_->addUpFlowForwarder(ff_dns_);
                ff_udp_high_->addUpFlowForwarder(ff_udp_generic_);
        }
}

void StackMobile::enableLinkLayerTagging(std::string type) {

        if (type.compare("vlan") == 0) {
                mux_eth_->addUpMultiplexer(mux_vlan_,ETHERTYPE_VLAN);
                mux_vlan_->addDownMultiplexer(mux_eth_);
                mux_vlan_->addUpMultiplexer(mux_ip_low_,ETHERTYPE_IP);
                mux_ip_low_->addDownMultiplexer(mux_vlan_);
        } else {
                if (type.compare("mpls") == 0) {
                        mux_eth_->addUpMultiplexer(mux_mpls_,ETHERTYPE_MPLS);
                	mux_mpls_->addDownMultiplexer(mux_eth_);
                        mux_mpls_->addUpMultiplexer(mux_ip_low_,ETHERTYPE_IP);
                        mux_ip_low_->addDownMultiplexer(mux_mpls_);
                } else {
#ifdef HAVE_LIBLOG4CXX
                        LOG4CXX_WARN (logger, "Unknown tagging type " << type );
#else
                        std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now();
                        std::time_t now = std::chrono::system_clock::to_time_t(time_point);
#ifdef __clang__
                        std::cout << "[" << std::put_time(std::localtime(&now), "%D %X") << "] ";
#else
                        char mbstr[100];
                        std::strftime(mbstr, 100, "%D %X", std::localtime(&now));
                        std::cout << "[" << mbstr << "] ";
#endif
                        std::cout << "Unknown tagging type " << type << std::endl;
#endif
                }
        }
}

void StackMobile::setFlowsTimeout(int timeout) {

        flow_table_tcp_->setTimeout(timeout);
        flow_table_udp_high_->setTimeout(timeout);
}

} // namespace aiengine
