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
 */
#include "UDPProtocol.h"
#include <iomanip> // setw

namespace aiengine {

void UDPProtocol::statistics(std::basic_ostream<char>& out) {

	if (stats_level_ > 0) {
		out << getName() << "(" << this << ") statistics" << std::dec << std::endl;
		out << "\t" << "Total packets:          " << std::setw(10) << total_packets_ <<std::endl;
		out << "\t" << "Total bytes:        " << std::setw(14) << total_bytes_ <<std::endl;
		if (stats_level_ > 1){ 
			out << "\t" << "Total validated packets:" << std::setw(10) << total_validated_packets_ <<std::endl;
			out << "\t" << "Total malformed packets:" << std::setw(10) << total_malformed_packets_ <<std::endl;
			if (stats_level_ > 2) {
				if(mux_.lock())
					mux_.lock()->statistics(out);
				if(flow_forwarder_.lock())
					flow_forwarder_.lock()->statistics(out);
				if (stats_level_ > 3){ 
					if(flow_table_)
						flow_table_->statistics(out);
					if(flow_cache_)
						flow_cache_->statistics(out);
				 }
			}
		}
	}
}

SharedPointer<Flow> UDPProtocol::getFlow(const Packet& packet) { 

	SharedPointer<Flow> flow; 

	if (flow_table_) {
		MultiplexerPtrWeak downmux = mux_.lock()->getDownMultiplexer();	
		MultiplexerPtr ipmux = downmux.lock();

                unsigned long h1 = ipmux->address.getHash(getSrcPort(),IPPROTO_UDP,getDstPort());
                unsigned long h2 = ipmux->address.getHash(getDstPort(),IPPROTO_UDP,getSrcPort());

		if (packet.haveTag() == true) {
			h1 = h1 ^ packet.getTag();
			h2 = h2 ^ packet.getTag();
		}

		flow = flow_table_->findFlow(h1,h2);
		if (!flow){
			if (flow_cache_){
				flow = flow_cache_->acquireFlow().lock();
				if (flow) {
					flow->setId(h1);
					if (packet.haveTag() == true) { 
						flow->setTag(packet.getTag());
					}

					// The time of the flow must be insert on the FlowManager table
					// in order to keep the index updated
                        		flow->setArriveTime(packet_time_);
                        		flow->setLastPacketTime(packet_time_);

                                        if (ipmux->address.getType() == 4) {
                                                flow->setFiveTuple(ipmux->address.getSourceAddress(),
                                                        getSrcPort(),IPPROTO_UDP,
                                                        ipmux->address.getDestinationAddress(),
                                                        getDstPort());
                                        } else {
                                                flow->setFiveTuple6(ipmux->address.getSourceAddress6(),
                                                        getSrcPort(),IPPROTO_UDP,
                                                        ipmux->address.getDestinationAddress6(),
                                                        getDstPort());
                                        }
					flow_table_->addFlow(flow);		
#if defined(PYTHON_BINDING) && defined(HAVE_ADAPTOR)
                        		if (getPythonObjectIsSet()) { // There is attached a database object
						databaseAdaptorInsertHandler(flow.get()); 
                        		}
#endif
				}
			}
		}
	}
	return flow; 
}

void UDPProtocol::processPacket(Packet& packet) {

	packet_time_ = packet.getPacketTime();
	SharedPointer<Flow> flow = getFlow(packet);

	current_flow_ = flow.get();

	++total_packets_;

	if(flow) {
		int bytes = (getLength() - getHeaderLength());

		if (bytes > packet.getLength()) { // The length of the packet is corrupted or not valid
			bytes = packet.getLength();
			flow->setPacketAnomaly(PacketAnomaly::UDP_BOGUS_HEADER);
		}

		total_bytes_ += bytes;
		flow->total_bytes += bytes;
		++flow->total_packets;
#ifdef DEBUG
                char mbstr[100];
                std::strftime(mbstr, 100, "%D %X", std::localtime(&packet_time_));
                std::cout << __FILE__ << ":" << __func__ << ": flow(" << current_flow_ << ")[" << mbstr << "] pkts:" << flow->total_packets;
                std::cout << " bytes:" << bytes << " pktlen:" << packet.getLength() << std::endl;
#endif

		if (flow->getPacketAnomaly() == PacketAnomaly::NONE) {
			flow->setPacketAnomaly(packet.getPacketAnomaly());
		}

		if(flow_forwarder_.lock()&&(bytes>0)) {
			FlowForwarderPtr ff = flow_forwarder_.lock();

                        // Modify the packet for the next level
                        packet.setPayload(&packet.getPayload()[getHeaderLength()]);
                        packet.setPrevHeaderSize(getHeaderLength());
                        packet.setPayloadLength(packet.getLength() - getHeaderLength());
                        packet.setDestinationPort(getDstPort());
                        packet.setSourcePort(getSrcPort());

                        flow->packet = const_cast<Packet*>(&packet);
                        ff->forwardFlow(flow.get());
		}
		
		if (flow->total_packets == 1) { // Just need to check once per flow
			if(ipset_mng_) {
				if (ipset_mng_->lookupIPAddress(flow->getDstAddrDotNotation())) {
					SharedPointer<IPAbstractSet> ipset = ipset_mng_->getMatchedIPSet();
					flow->ipset = ipset;
#ifdef DEBUG
                                        std::cout << __PRETTY_FUNCTION__ << ":flow:" << flow << ":Lookup positive on IPSet:" << ipset->getName() << std::endl;
#endif
#ifdef PYTHON_BINDING
                                        if (ipset->haveCallback()) {
						ipset->executeCallback(flow.get());
                                       	}
#endif
				}
			}	
		}

#if defined(PYTHON_BINDING) && defined(HAVE_ADAPTOR) 
		if (((flow->total_packets - 1) % getPacketSampling()) == 0 ) {
			if (getPythonObjectIsSet()) { // There is attached a database object
				databaseAdaptorUpdateHandler(flow.get());
			} 
		}
#endif
		// Check if we need to update the timers of the flow manager
		if ((packet_time_ - flow_table_->getTimeout()) > last_timeout_ ) {
			last_timeout_ = packet_time_;
			flow_table_->updateTimers(packet_time_);
		}
		flow->setLastPacketTime(packet_time_);
	}

}

} // namespace aiengine