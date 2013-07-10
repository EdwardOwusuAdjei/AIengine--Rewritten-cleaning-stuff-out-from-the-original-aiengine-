#include "Stack3G.h"

Stack3G::Stack3G()
{
	name_ = "3G Network Stack";

	// Allocate all the Protocol objects
        eth_= EthernetProtocolPtr(new EthernetProtocol());
        ip_low_ = IPProtocolPtr(new IPProtocol());
	ip_high_ = IPProtocolPtr(new IPProtocol());
        udp_low_ = UDPProtocolPtr(new UDPProtocol());
	udp_high_ = UDPProtocolPtr(new UDPProtocol());
        tcp_ = TCPProtocolPtr(new TCPProtocol());
        gprs_ = GPRSProtocolPtr(new GPRSProtocol());
        icmp_ = ICMPProtocolPtr(new ICMPProtocol());
        http_ = HTTPProtocolPtr(new HTTPProtocol());
        ssl_ = SSLProtocolPtr(new SSLProtocol());
        dns_ = DNSProtocolPtr(new DNSProtocol());

	// Allocate the Multiplexers
	mux_eth_ = MultiplexerPtr(new Multiplexer());
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
        flow_mng_tcp_ = FlowManagerPtr(new FlowManager());
        flow_mng_udp_high_ = FlowManagerPtr(new FlowManager());
        flow_mng_udp_low_ = FlowManagerPtr(new FlowManager());

	ff_tcp_ = FlowForwarderPtr(new FlowForwarder());
	ff_udp_low_ = FlowForwarderPtr(new FlowForwarder());
	ff_udp_high_ = FlowForwarderPtr(new FlowForwarder());
	ff_http_ = FlowForwarderPtr(new FlowForwarder());
	ff_ssl_ = FlowForwarderPtr(new FlowForwarder());
	ff_dns_ = FlowForwarderPtr(new FlowForwarder());
	ff_gprs_ = FlowForwarderPtr(new FlowForwarder());

	//configure the Ethernet Layer 
	eth_->setMultiplexer(mux_eth_);
	mux_eth_->setProtocol(static_cast<ProtocolPtr>(eth_));
	mux_eth_->setProtocolIdentifier(0);
	mux_eth_->setHeaderSize(eth_->getHeaderSize());
	mux_eth_->addChecker(std::bind(&EthernetProtocol::ethernetChecker,eth_,std::placeholders::_1));

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
	mux_udp_low_->addDownMultiplexer(mux_ip_high_);

	// Connect the FlowManager and FlowCache
	tcp_->setFlowCache(flow_cache_tcp_);
	tcp_->setFlowManager(flow_mng_tcp_);
			
	udp_low_->setFlowCache(flow_cache_udp_low_);
	udp_low_->setFlowManager(flow_mng_udp_low_);
	udp_high_->setFlowCache(flow_cache_udp_low_);
	udp_high_->setFlowManager(flow_mng_udp_low_);
	
	//udp_high_->setFlowCache(flow_cache_udp_high_);
	//udp_high_->setFlowManager(flow_mng_udp_high_);

	// Configure the FlowForwarders
	udp_low_->setFlowForwarder(ff_udp_low_);
	ff_udp_low_->addUpFlowForwarder(ff_gprs_);

	tcp_->setFlowForwarder(ff_tcp_);	
	udp_high_->setFlowForwarder(ff_udp_high_);	
	
	ff_tcp_->addUpFlowForwarder(ff_http_);
	ff_tcp_->addUpFlowForwarder(ff_ssl_);
	ff_udp_high_->addUpFlowForwarder(ff_dns_);
}

void Stack3G::statistics(std::basic_ostream<char>& out)
{
	
	eth_->statistics(out);
	out << std::endl;
	ip_low_->statistics(out);
	out << std::endl;
	udp_low_->statistics(out);
	out << std::endl;
	ip_high_->statistics(out);
	out << std::endl;
	tcp_->statistics(out);
	out << std::endl;
	udp_high_->statistics(out);
	out << std::endl;
	icmp_->statistics(out);
	out << std::endl;
	http_->statistics(out);
	out << std::endl;
	ssl_->statistics(out);
	out << std::endl;
	dns_->statistics(out);
}

void Stack3G::printFlows(std::basic_ostream<char>& out)
{
	out << "Flows on memory" << std::endl;
	flow_mng_udp_low_->printFlows(out);
	flow_mng_tcp_->printFlows(out);
	flow_mng_udp_high_->printFlows(out);
}

