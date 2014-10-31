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
#include "SSLProtocol.h"
#include <iomanip> // setw

namespace aiengine {

#ifdef HAVE_LIBLOG4CXX
log4cxx::LoggerPtr SSLProtocol::logger(log4cxx::Logger::getLogger("aiengine.ssl"));
#endif

void SSLProtocol::releaseCache() {

        FlowManagerPtr fm = flow_mng_.lock();

        if (fm) {
                auto ft = fm->getFlowTable();

                std::ostringstream msg;
                msg << "Releasing " << getName() << " cache";

                infoMessage(msg.str());

		int64_t total_bytes_released = 0;
		int64_t total_bytes_released_by_flows = 0;
                int32_t release_flows = 0;
                int32_t release_hosts = host_map_.size();

                // Compute the size of the strings used as keys on the map
                std::for_each (host_map_.begin(), host_map_.end(), [&total_bytes_released] (std::pair<std::string,HostHits> const &ht) {
                        total_bytes_released += ht.first.size();
                });

                for (auto &flow: ft) {
                        SharedPointer<SSLHost> host = flow->ssl_host.lock();

                        if (host) { // The flow have a host attatched
                                flow->ssl_host.reset();
				total_bytes_released_by_flows += host->getName().size();
                                host_cache_->release(host);
				++release_flows;
                        }
                } 
                host_map_.clear();

                double cache_compression_rate = 0;

                if (total_bytes_released > 0 ) {
                        cache_compression_rate = 100 - ((total_bytes_released*100)/total_bytes_released_by_flows);
                }
        
        	msg.str("");
                msg << "Release " << release_hosts << " hosts, " << release_flows << " flows";
		msg << ", " << total_bytes_released << " bytes, compression rate " << cache_compression_rate << "%";
                infoMessage(msg.str());
        }
}


void SSLProtocol::attach_host_to_flow(Flow *flow, std::string &servername) {

	SharedPointer<SSLHost> host_ptr = flow->ssl_host.lock();

	if (!host_ptr) { // There is no Host object attached to the flow
		HostMapType::iterator it = host_map_.find(servername);
		if (it == host_map_.end()) {
			host_ptr = host_cache_->acquire().lock();
			if (host_ptr) {
				host_ptr->setName(servername);
				flow->ssl_host = host_ptr;

				host_map_.insert(std::make_pair(servername,std::make_pair(host_ptr,1)));
			}
		} else {
			int *counter = &std::get<1>(it->second);
			++(*counter);
			flow->ssl_host = std::get<0>(it->second);
		}
	}
}


void SSLProtocol::handle_client_hello(Flow *flow,int offset, u_char *data) {

	int payload_length = flow->packet->getLength();
	ssl_hello *hello = reinterpret_cast<ssl_hello*>(data); 
	uint16_t version = ntohs(hello->version);
	int block_offset = sizeof(ssl_hello) + offset;

	++ total_client_hellos_;

	if((version >= SSL3_VERSION)and(version <= TLS1_2_VERSION)) {

		if (ntohs(hello->session_id_length) > 0) {
			// Session id management
			// the alignment of the struct should be fix
			block_offset += 32;
		}

		uint16_t cipher_length = ntohs((data[block_offset+1] << 8) + data[block_offset]);
		if (cipher_length < payload_length) {

			block_offset += cipher_length  + 2;
			u_char *compression_pointer = &data[block_offset];
			short compression_length = compression_pointer[0];
			
			if(compression_length > 0) {
				block_offset += compression_length + 2;
			}
			if (block_offset < payload_length) {
				u_char *extensions = &data[block_offset];
				uint16_t extensions_length = ((extensions[1] << 8) + extensions[0]);
				if (extensions_length + block_offset < payload_length) {
					block_offset += 2;
					extensions = &data[block_offset];
					uint16_t extension_type = ((extensions[1] << 8) + extensions[0]);
					short extension_length __attribute__((unused)) = extensions[2];

					if (extension_type == 0x0000) { // Server name
						//block_offset += 2;
						ssl_server_name *server = reinterpret_cast<ssl_server_name*>(&extensions[3]);
						int server_length = ntohs(server->length);
						if ((block_offset + server_length < payload_length )and(server_length > 0)) {
							std::string servername((char*)server->data,0,server_length);
							DomainNameManagerPtr ban_dnm = ban_host_mng_.lock();

							ban_dnm = ban_host_mng_.lock();
							if (ban_dnm) {
								SharedPointer<DomainName> host_candidate = ban_dnm->getDomainName(servername);
								if (host_candidate) {
#ifdef HAVE_LIBLOG4CXX
									LOG4CXX_INFO (logger, "Flow:" << *flow << " matchs with banned host " << host_candidate->getName());
#endif
									++total_ban_hosts_;
									return;
								}
							}
							++total_allow_hosts_;

							attach_host_to_flow(flow,servername);
						}	
					} // Server name 
				}
			}	
		}
	} // end version 
}

void SSLProtocol::handle_server_hello(Flow *flow,int offset,unsigned char *data) {

	ssl_hello *hello __attribute__((unused)) = reinterpret_cast<ssl_hello*>(data); 
	++ total_server_hellos_;
}

void SSLProtocol::handle_certificate(Flow *flow,int offset, unsigned char *data) {

	++ total_certificates_;
}

void SSLProtocol::processFlow(Flow *flow) {

	++total_packets_;
	total_bytes_ += flow->packet->getLength();
	++flow->total_packets_l7;

	if (flow->total_packets_l7 < 3) { 
		setHeader(flow->packet->getPayload());

		int length = ntohs(ssl_header_->length);
		if (length > 0) {
			ssl_record *record = ssl_header_;
			int offset = 0;		// Total offset byte
			int maxattemps = 0; 	// For prevent invalid decodings

			do {
				uint16_t version = ntohs(record->version);
				int block_length = ntohs(record->length);
				short type = record->data[0];
				++maxattemps;

				if((version >= SSL3_VERSION)and(version <= TLS1_2_VERSION)) {
					// This is a valid SSL header that we could extract some usefulll information.
					// SSL Records are group by blocks
					u_char *ssl_data = record->data;
					bool have_data = false;

					if (type == SSL3_MT_CLIENT_HELLO)  {
						handle_client_hello(flow,offset,ssl_data);
						have_data = true;
					} else if (type == SSL3_MT_SERVER_HELLO)  {
						handle_server_hello(flow,offset,ssl_data);
						have_data = true;
					} else if (type == SSL3_MT_CERTIFICATE) {
						handle_certificate(flow,offset,ssl_data);
						have_data = true;
					}

					if (have_data) {
						++ total_records_;
						offset += block_length;
						ssl_data = &(record->data[block_length]);
						block_length = ntohs(record->length);
					}

					record = reinterpret_cast<ssl_record*>(ssl_data);
					offset += 5;	
				} else {
					break;
				}
				if (maxattemps == 4 ) break;
			}while(offset < flow->packet->getLength());

			DomainNameManagerPtr host_mng = host_mng_.lock();
			if (host_mng) {
				SharedPointer<SSLHost> host_name = flow->ssl_host.lock();

				// TODO: just handled the client hello, so there is no need of checking on packetsl7 > than 1
				if ((host_name)and(flow->total_packets_l7 == 1)) {
					SharedPointer<DomainName> host_candidate = host_mng->getDomainName(host_name->getName());
					
					if (host_candidate) {
#ifdef PYTHON_BINDING
#ifdef HAVE_LIBLOG4CXX
						LOG4CXX_INFO (logger, "Flow:" << *flow << " matchs with " << host_candidate->getName());
#endif  
						if(host_candidate->haveCallback()) {
							host_candidate->executeCallback(flow);
						}
#endif
					}
				}
			}
		}
	}
}

void SSLProtocol::statistics(std::basic_ostream<char>& out) {

	if (stats_level_ > 0) {
		out << getName() << "(" << this << ") statistics" << std::dec << std::endl;
		out << "\t" << "Total packets:          " << std::setw(10) << total_packets_ <<std::endl;
		out << "\t" << "Total bytes:        " << std::setw(14) << total_bytes_ <<std::endl;
		if (stats_level_ > 1) { 
			out << "\t" << "Total validated packets:" << std::setw(10) << total_validated_packets_ <<std::endl;
			out << "\t" << "Total malformed packets:" << std::setw(10) << total_malformed_packets_ <<std::endl;
			if(stats_level_ > 3) {
			
				out << "\t" << "Total client hellos:    " << std::setw(10) << total_client_hellos_ <<std::endl;
				out << "\t" << "Total server hellos:    " << std::setw(10) << total_server_hellos_ <<std::endl;
				out << "\t" << "Total certificates:     " << std::setw(10) << total_certificates_ <<std::endl;
				out << "\t" << "Total records:          " << std::setw(10) << total_records_ <<std::endl;
				out << "\t" << "Total allow hosts:      " << std::setw(10) << total_allow_hosts_ <<std::endl;
				out << "\t" << "Total banned hosts:     " << std::setw(10) << total_ban_hosts_ <<std::endl;
			}
			if (stats_level_ > 2) {
				if(flow_forwarder_.lock())
					flow_forwarder_.lock()->statistics(out);
			}
			if (stats_level_ > 3) {
				host_cache_->statistics(out);
				if(stats_level_ > 4) {
					out << "\tSSL Hosts usage" << std::endl;

                                        std::vector<std::pair<std::string,HostHits>> h_list(host_map_.begin(),host_map_.end());
                                        // Sort The host_map by using lambdas
                                        std::sort(
                                        	h_list.begin(),
                                                h_list.end(),
                                                [](std::pair<std::string,HostHits> const &a,
                                                std::pair<std::string,HostHits> const &b)
                                        {
                                                int v1 = std::get<1>(a.second);
                                                int v2 = std::get<1>(b.second);

                                                return v1 > v2;
                                        });

					for(auto it = h_list.begin(); it!=h_list.end(); ++it) {
						SharedPointer<SSLHost> host = std::get<0>((*it).second);
						int count = std::get<1>((*it).second);
						if(host)
							out << "\t\tHost:" << host->getName() <<":" << count << std::endl;
					}
				}
			}
		}
	}
}

} // namespace aiengine