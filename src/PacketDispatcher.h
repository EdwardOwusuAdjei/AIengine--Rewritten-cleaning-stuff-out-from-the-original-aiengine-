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
#ifndef SRC_PACKETDISPATCHER_H_
#define SRC_PACKETDISPATCHER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pcap.h>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#ifdef HAVE_LIBLOG4CXX
#include "log4cxx/logger.h"
#endif
#include "NetworkStack.h"
#include "Multiplexer.h"
#include "./ethernet/EthernetProtocol.h"
#include "Protocol.h"
#include "StackLan.h"
#include "StackMobile.h"
#include <sys/resource.h>

namespace aiengine {

#define PACKET_RECVBUFSIZE    2048        /// receive_from buffer size for a single datagram

#define BOOST_ASIO_DISABLE_EPOLL

typedef boost::asio::posix::stream_descriptor PcapStream;
typedef std::shared_ptr<PcapStream> PcapStreamPtr;

class PacketDispatcher 
{
public:
	class Statistics
	{
		public:
			explicit Statistics():interval(0),prev_total_packets_per_interval(0) {
			
				ru_utime.tv_sec = 0; ru_utime.tv_usec = 0; 
				ru_stime.tv_sec = 0; ru_stime.tv_usec = 0; 
			}
			virtual ~Statistics() {}
			int interval;
			struct timeval ru_utime;
			struct timeval ru_stime;
			int64_t prev_total_packets_per_interval;	
	};

    	explicit PacketDispatcher():
		io_service_(),
		stats_(),
		idle_work_interval_(5),
		idle_work_(io_service_,boost::posix_time::seconds(0)),
		total_packets_(0),
		total_bytes_(0),
		pcap_file_ready_(false),
		device_is_ready_(false) {
	
		setIdleFunction(std::bind(&PacketDispatcher::default_idle_function,this));
	}

    	virtual ~PacketDispatcher() { io_service_.stop(); }

	void openDevice(std::string device);
	void closeDevice();
	void openPcapFile(std::string filename);
	void closePcapFile();

	void stop() { io_service_.stop();}
	void run(); 
	void runPcap(); 

	uint64_t getTotalBytes() const { return total_bytes_;}
	uint64_t getTotalPackets() const { return total_packets_;}

	void setStack(NetworkStackPtr stack) { setDefaultMultiplexer(stack->getLinkLayerMultiplexer().lock());}
	void setStack(StackLan& stack) { setDefaultMultiplexer(stack.getLinkLayerMultiplexer().lock());}
	void setStack(StackMobile& stack) { setDefaultMultiplexer(stack.getLinkLayerMultiplexer().lock());}

	void setDefaultMultiplexer(MultiplexerPtr mux); // just use for the unit tests
	void setIdleFunction(std::function <void ()> idle_function) { idle_function_ = idle_function;}
private:
	void start_operations();
	void handle_receive(boost::system::error_code error);
	void do_read(boost::system::error_code error);
	void forwardRawPacket(unsigned char *packet,int length);
	void idle_handler(boost::system::error_code error);
	void default_idle_function() const {};

#ifdef HAVE_LIBLOG4CXX
	static log4cxx::LoggerPtr logger;
#endif
	PcapStreamPtr stream_;
	bool pcap_file_ready_;
	bool read_in_progress_;
	bool device_is_ready_;

	uint64_t total_packets_;	
	uint64_t total_bytes_;	
    	pcap_t* pcap_;
	boost::asio::io_service io_service_;
	boost::asio::deadline_timer idle_work_;
	int idle_work_interval_;
	Statistics stats_;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	std::function <void ()> idle_function_;

	EthernetProtocolPtr eth_;	
	Packet current_packet_;
	MultiplexerPtr defMux_;
};

typedef std::shared_ptr<PacketDispatcher> PacketDispatcherPtr;

} // namespace aiengine

#endif  // SRC_PACKETDISPATCHER_H_
