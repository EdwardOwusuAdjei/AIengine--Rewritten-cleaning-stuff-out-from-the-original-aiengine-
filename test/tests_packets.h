#ifndef _tests_packets_H_
#define _tests_packets_H_

// A Mini database of different packets

// localip 192.168.1.25
// remoteip 66.220.153.28
// ttl = 128
// tcp syn packet

char *raw_packet_ip_tcp_syn =
	"\x45\x00"
	"\x00\x34\x8b\x1e\x40\x00\x80\x06\xd1\xeb\xc0\xa8\x01\x19\x42\xdc"
	"\x99\x1c\x05\xb1\x00\x50\x06\xa4\x1a\x34\x00\x00\x00\x00\x80\x02"
	"\xff\xff\xaa\x84\x00\x00\x02\x04\x05\xb4\x01\x03\x03\x01\x01\x01"
	"\x04\x02";
int raw_packet_ip_tcp_syn_length = 52;

// ethernet, ip, udp and dns
char *raw_packet_ethernet_ip_udp_dns =
	"\x00\x0c\x29\x2e\x3c\x2a\x90\x84\x0d\x62\xd8\x04\x08\x00\x45\x00"
	"\x00\x3d\x8a\x0d\x00\x00\xec\x11\xf4\x4f\xc0\xa8\x01\x76\x50\x3a"
	"\x3d\xfa\xe9\xb3\x00\x35\x00\x29\x05\x94\x84\xd3\x01\x00\x00\x01"
	"\x00\x00\x00\x00\x00\x00\x02\x63\x68\x04\x70\x6f\x6f\x6c\x03\x6e"
	"\x74\x70\x03\x6f\x72\x67\x00\x00\x01\x00\x01";
int raw_packet_ethernet_ip_udp_dns_length = 75;
	
// ethernet, vlan, ip, udp and dns
char *raw_packet_ethernet_vlan_ip_udp_dns =
	"\x00\x0c\x29\x2e\x3c\x2a\x90\x84\x0d\x62\xd8\x04" "\x81\x00\x02\x5e\x08\x00"
	"\x45\x00"
	"\x00\x3d\x8a\x0d\x00\x00\xec\x11\xf4\x4f\xc0\xa8\x01\x76\x50\x3a"
	"\x3d\xfa\xe9\xb3\x00\x35\x00\x29\x05\x94\x84\xd3\x01\x00\x00\x01"
	"\x00\x00\x00\x00\x00\x00\x02\x63\x68\x04\x70\x6f\x6f\x6c\x03\x6e"
	"\x74\x70\x03\x6f\x72\x67\x00\x00\x01\x00\x01";
int raw_packet_ethernet_vlan_ip_udp_dns_length = 79;
	

// ethernet, ip1 , ip2 , udp an dns
// ip1.ttl = 64
// ip2.src = 192.168.1.118
// ip2.dst = 80.58.61.250
 
char *raw_packet_ethernet_ip_ip_udp_dns =
	"\x00\x0c\x29\x2e\x3c\x2a\x90\x84\x0d\x62\xd8\x04" "\x08\x00"
	// IP
	"\x45\x00\x00\x51\x00\x00\x40\x00\x40" "\x04" "\xd5\x57\x0a\x3a\x09\x76"
	"\xc3\x72\x8d\xd1"
	// IP
        "\x45\x00" // 36
        "\x00\x3d\x8a\x0d\x00\x00\xec\x11\xf4\x4f\xc0\xa8\x01\x76\x50\x3a" 
        "\x3d\xfa"
	/* udp */
	"\xe9\xb3\x00\x35\x00\x29\x05\x94"
	// dns
	"\x84\xd3\x01\x00\x00\x01"
        "\x00\x00\x00\x00\x00\x00\x02\x63\x68\x04\x70\x6f\x6f\x6c\x03\x6e" // 84
        "\x74\x70\x03\x6f\x72\x67\x00\x00\x01\x00\x01"; // 95 
int raw_packet_ethernet_ip_ip_udp_dns_length = 95;

// ethernet, ip, udp, dhcp offer
char *raw_packet_ethernet_ip_udp_dhcp_offer =       
        "\x00\x0c\x29\xc0\xb9\xa8\x00\x50\x56\xfa\xa0\x55\x08\x00\x45\x10"
        "\x01\x48\x00\x00\x00\x00\x10\x11\xd6\xbd\xc0\xa8\x28\xfe\xc0\xa8"
        "\x28\x89\x00\x43\x00\x44\x01\x34\xe3\xaa\x02\x01\x06\x00\x66\xc8"
        "\x02\x4a\x00\x00\x00\x00\x00\x00\x00\x00\xc0\xa8\x28\x89\xc0\xa8"
        "\x28\xfe\x00\x00\x00\x00\x00\x0c\x29\xc0\xb9\xa8\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x63\x82\x53\x63\x35\x01\x02\x36\x04\xc0"
        "\xa8\x28\xfe\x33\x04\x00\x00\x07\x08\x01\x04\xff\xff\xff\x00\x1c"
        "\x04\xc0\xa8\x28\xff\x03\x04\xc0\xa8\x28\x02\x0f\x0b\x6c\x6f\x63"
        "\x61\x6c\x64\x6f\x6d\x61\x69\x6e\x06\x04\xc0\xa8\x28\x02\x2c\x04"
        "\xc0\xa8\x28\x02\xff\x00";
int raw_packet_ethernet_ip_udp_dhcp_offer_length= 342; 


// ip.src = 192.168.40.254
// ip.dst = 192.168.40.137
// ip.ttl = 128
char *raw_packet_ethernet_ip_icmp_echo_request = 
	"\x00\x0c\x29\xc0\xb9\xa8\x00\x50\x56\xef\xaf\xd8\x08\x00\x45\x00"
	"\x00\x30\x34\xb9\x00\x00\x80\x01\x33\x3c\xc0\xa8\x28\xfe\xc0\xa8"
	"\x28\x89\x08\x00\xd3\xb5\xd8\x05\x00\x00\xdb\x01\x92\x7c\x83\x4d"
	"\x13\x78\x00\x00\x3b\x00\x00\x00\x00\x00\x0d\x00\x00\x00";
int raw_packet_ethernet_ip_icmp_echo_request_length = 62;

// ip.src = 192.168.40.137
// ip.dst = 192.168.40.254
// ip.ttl = 64
char *raw_packet_ethernet_ip_icmp_echo_reply = 
	"\x00\x50\x56\xfa\xa0\x55\x00\x0c\x29\xc0\xb9\xa8\x08\x00\x45\x00"
	"\x00\x30\xa0\xbf\x00\x00\x40\x01\x07\x36\xc0\xa8\x28\x89\xc0\xa8"
	"\x28\xfe\x00\x00\xdb\xb5\xd8\x05\x00\x00\xdb\x01\x92\x7c\x83\x4d"
	"\x13\x78\x00\x00\x3b\x00\x00\x00\x00\x00\x0d\x00\x00\x00";
int raw_packet_ethernet_ip_icmp_echo_reply_length = 62;

// http request to google 
//GET /pagead/conversion/1053964783/?random=1297767800114&cv=6&fst=1297767799966&num=2&fmt=1&label=eyRdCNmRThDv88j2Aw&bg=ffffff&hl=en&gl=US&guid=ON&u_h=768&u_w=1024&u_ah=720&u_aw=1024&u_cd=24&u_his=5&u_tz=60&u_java=false&u_nplug=5&u_nmime=39&ref=http%3A//www.google.com/chrome/eula.html%3Fhl%3Des&url=http%3A//www.google.com/chrome/thankyou.html%3Fhl%3Des HTTP/1.1
// Host: www.googleadservices.com
// User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.2.3) Gecko/20100423 Ubuntu/10.04 (lucid) Firefox/3.6.3
// Accept: image/png,image/*;q=0.8,*/*;q=0.5
// Accept-Language: es-es,es;q=0.8,en-us;q=0.5,en;q=0.3
// Accept-Encoding: gzip,deflate
// Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7
// Keep-Alive: 115
// Connection: keep-alive
// Referer: http://www.google.com/chrome/thankyou.html?hl=es
//

char *raw_packet_ethernet_ip_tcp_http_get = 
"\x00\x50\x56\xef\xaf\xd8\x00\x0c\x29\xc0\xb9\xa8\x08\x00\x45\x00"
"\x03\x3d\x97\x52\x40\x00\x40\x06\x00\x81\xc0\xa8\x28\x89\xd1\x55"
"\xe5\x60\xd1\x85\x00\x50\xc1\x96\xd5\xfc\x71\xf8\x2e\xf3\x50\x18"
"\x16\xd0\x1b\x72\x00\x00\x47\x45\x54\x20\x2f\x70\x61\x67\x65\x61"
"\x64\x2f\x63\x6f\x6e\x76\x65\x72\x73\x69\x6f\x6e\x2f\x31\x30\x35"
"\x33\x39\x36\x34\x37\x38\x33\x2f\x3f\x72\x61\x6e\x64\x6f\x6d\x3d"
"\x31\x32\x39\x37\x37\x36\x37\x38\x30\x30\x31\x31\x34\x26\x63\x76"
"\x3d\x36\x26\x66\x73\x74\x3d\x31\x32\x39\x37\x37\x36\x37\x37\x39"
"\x39\x39\x36\x36\x26\x6e\x75\x6d\x3d\x32\x26\x66\x6d\x74\x3d\x31"
"\x26\x6c\x61\x62\x65\x6c\x3d\x65\x79\x52\x64\x43\x4e\x6d\x52\x54"
"\x68\x44\x76\x38\x38\x6a\x32\x41\x77\x26\x62\x67\x3d\x66\x66\x66"
"\x66\x66\x66\x26\x68\x6c\x3d\x65\x6e\x26\x67\x6c\x3d\x55\x53\x26"
"\x67\x75\x69\x64\x3d\x4f\x4e\x26\x75\x5f\x68\x3d\x37\x36\x38\x26"
"\x75\x5f\x77\x3d\x31\x30\x32\x34\x26\x75\x5f\x61\x68\x3d\x37\x32"
"\x30\x26\x75\x5f\x61\x77\x3d\x31\x30\x32\x34\x26\x75\x5f\x63\x64"
"\x3d\x32\x34\x26\x75\x5f\x68\x69\x73\x3d\x35\x26\x75\x5f\x74\x7a"
"\x3d\x36\x30\x26\x75\x5f\x6a\x61\x76\x61\x3d\x66\x61\x6c\x73\x65"
"\x26\x75\x5f\x6e\x70\x6c\x75\x67\x3d\x35\x26\x75\x5f\x6e\x6d\x69"
"\x6d\x65\x3d\x33\x39\x26\x72\x65\x66\x3d\x68\x74\x74\x70\x25\x33"
"\x41\x2f\x2f\x77\x77\x77\x2e\x67\x6f\x6f\x67\x6c\x65\x2e\x63\x6f"
"\x6d\x2f\x63\x68\x72\x6f\x6d\x65\x2f\x65\x75\x6c\x61\x2e\x68\x74"
"\x6d\x6c\x25\x33\x46\x68\x6c\x25\x33\x44\x65\x73\x26\x75\x72\x6c"
"\x3d\x68\x74\x74\x70\x25\x33\x41\x2f\x2f\x77\x77\x77\x2e\x67\x6f"
"\x6f\x67\x6c\x65\x2e\x63\x6f\x6d\x2f\x63\x68\x72\x6f\x6d\x65\x2f"
"\x74\x68\x61\x6e\x6b\x79\x6f\x75\x2e\x68\x74\x6d\x6c\x25\x33\x46"
"\x68\x6c\x25\x33\x44\x65\x73\x20\x48\x54\x54\x50\x2f\x31\x2e\x31"
"\x0d\x0a\x48\x6f\x73\x74\x3a\x20\x77\x77\x77\x2e\x67\x6f\x6f\x67"
"\x6c\x65\x61\x64\x73\x65\x72\x76\x69\x63\x65\x73\x2e\x63\x6f\x6d"
"\x0d\x0a\x55\x73\x65\x72\x2d\x41\x67\x65\x6e\x74\x3a\x20\x4d\x6f"
"\x7a\x69\x6c\x6c\x61\x2f\x35\x2e\x30\x20\x28\x58\x31\x31\x3b\x20"
"\x55\x3b\x20\x4c\x69\x6e\x75\x78\x20\x69\x36\x38\x36\x3b\x20\x65"
"\x73\x2d\x45\x53\x3b\x20\x72\x76\x3a\x31\x2e\x39\x2e\x32\x2e\x33"
"\x29\x20\x47\x65\x63\x6b\x6f\x2f\x32\x30\x31\x30\x30\x34\x32\x33"
"\x20\x55\x62\x75\x6e\x74\x75\x2f\x31\x30\x2e\x30\x34\x20\x28\x6c"
"\x75\x63\x69\x64\x29\x20\x46\x69\x72\x65\x66\x6f\x78\x2f\x33\x2e"
"\x36\x2e\x33\x0d\x0a\x41\x63\x63\x65\x70\x74\x3a\x20\x69\x6d\x61"
"\x67\x65\x2f\x70\x6e\x67\x2c\x69\x6d\x61\x67\x65\x2f\x2a\x3b\x71"
"\x3d\x30\x2e\x38\x2c\x2a\x2f\x2a\x3b\x71\x3d\x30\x2e\x35\x0d\x0a"
"\x41\x63\x63\x65\x70\x74\x2d\x4c\x61\x6e\x67\x75\x61\x67\x65\x3a"
"\x20\x65\x73\x2d\x65\x73\x2c\x65\x73\x3b\x71\x3d\x30\x2e\x38\x2c"
"\x65\x6e\x2d\x75\x73\x3b\x71\x3d\x30\x2e\x35\x2c\x65\x6e\x3b\x71"
"\x3d\x30\x2e\x33\x0d\x0a\x41\x63\x63\x65\x70\x74\x2d\x45\x6e\x63"
"\x6f\x64\x69\x6e\x67\x3a\x20\x67\x7a\x69\x70\x2c\x64\x65\x66\x6c"
"\x61\x74\x65\x0d\x0a\x41\x63\x63\x65\x70\x74\x2d\x43\x68\x61\x72"
"\x73\x65\x74\x3a\x20\x49\x53\x4f\x2d\x38\x38\x35\x39\x2d\x31\x2c"
"\x75\x74\x66\x2d\x38\x3b\x71\x3d\x30\x2e\x37\x2c\x2a\x3b\x71\x3d"
"\x30\x2e\x37\x0d\x0a\x4b\x65\x65\x70\x2d\x41\x6c\x69\x76\x65\x3a"
"\x20\x31\x31\x35\x0d\x0a\x43\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e"
"\x3a\x20\x6b\x65\x65\x70\x2d\x61\x6c\x69\x76\x65\x0d\x0a\x52\x65"
"\x66\x65\x72\x65\x72\x3a\x20\x68\x74\x74\x70\x3a\x2f\x2f\x77\x77"
"\x77\x2e\x67\x6f\x6f\x67\x6c\x65\x2e\x63\x6f\x6d\x2f\x63\x68\x72"
"\x6f\x6d\x65\x2f\x74\x68\x61\x6e\x6b\x79\x6f\x75\x2e\x68\x74\x6d"
"\x6c\x3f\x68\x6c\x3d\x65\x73\x0d\x0a\x0d\x0a";

int raw_packet_ethernet_ip_tcp_http_get_length = 843;

// grps packet with icmp
// source address 127.0.0.1
// source and dest adddress 192.168.0.1 and 192.168.0.3
// This packet is GTP version 0, DONT USE! BECAUSE IS NOT SUPPORTED

char *raw_packet_ethernet_ip_udp_gprs_ip_icmp_echo = 
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x45\x00"
"\x00\x84\x00\x00\x40\x00\x40\x11\x3c\x66\x7f\x00\x00\x02\x7f\x00"
"\x00\x01\x0d\x3a\x0d\x3a\x00\x70\xfe\x84\x1e\xff\x00\x54\x00\x00"
"\x00\x01\xff\xff\xff\xff\x42\x00\x01\x21\x43\x65\x87\x09\x45\x00"
"\x00\x54\x00\x00\x40\x00\x40\x01\xc5\x3f\xc0\xa8\x00\x03\xd1\x55"
"\xe3\x68\x08\x00\xe5\xe9\x00\x00\x00\x00\x82\x54\xf0\x4b\xaa\x72"
"\x0a\x00\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15"
"\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25"
"\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35"
"\x36\x37";

int raw_packet_ethernet_ip_udp_gprs_ip_icmp_echo_length = 146;

// ethernet ip tcp ssl packet
// source address 192.168.13 destination 74.125.24.186
// client hello

char *raw_packet_ethernet_ip_tcp_ssl_client_hello =
"\x04\x71\x7d\x00\xb8\x71\x1d\xa9\x00\x49\x43\x5c\x08\x00\x45\x00"
"\x00\xf5\x4b\x3a\x40\x00\x40\x06\xc9\xd9\xc0\xa8\x01\x0d\x4a\x7d"
"\x18\xbd\xac\xe9\x01\xbb\x15\x98\x88\x0d\xd0\xa5\x1a\x97\x80\x18"
"\x00\xe5\xec\xad\x00\x00\x01\x01\x08\x0a\x00\x11\xc3\x3e\x2d\x11"
"\x14\xe5\x16\x03\x01\x00\xbc\x01\x00\x00\xb8\x03\x02\x51\xbb\x49"
"\x3b\xb6\xee\x5a\xe1\x35\x52\x0e\x64\xfd\x6a\x93\x6d\xab\x1a\xb7"
"\xbd\xb4\x7f\xd0\x76\x05\x10\xbc\x02\xda\x2a\xb1\x1e\x00\x00\x48"
"\xc0\x0a\xc0\x14\x00\x88\x00\x87\x00\x39\x00\x38\xc0\x0f\xc0\x05"
"\x00\x84\x00\x35\xc0\x07\xc0\x09\xc0\x11\xc0\x13\x00\x45\x00\x44"
"\x00\x66\x00\x33\x00\x32\xc0\x0c\xc0\x0e\xc0\x02\xc0\x04\x00\x96"
"\x00\x41\x00\x05\x00\x04\x00\x2f\xc0\x08\xc0\x12\x00\x16\x00\x13"
"\xc0\x0d\xc0\x03\xfe\xff\x00\x0a\x01\x00\x00\x47\x00\x00\x00\x17"
"\x00\x15\x00\x00\x12\x30\x2e\x64\x72\x69\x76\x65\x2e\x67\x6f\x6f"
"\x67\x6c\x65\x2e\x63\x6f\x6d\xff\x01\x00\x01\x00\x00\x0a\x00\x08"
"\x00\x06\x00\x17\x00\x18\x00\x19\x00\x0b\x00\x02\x01\x00\x00\x23"
"\x00\x00\x33\x74\x00\x00\x75\x4f\x00\x00\x00\x05\x00\x05\x01\x00"
"\x00\x00\x00";

int raw_packet_ethernet_ip_tcp_ssl_client_hello_length = 259;



// access to barrapunto.es
// GET / HTTP/1.1
// Host: www.barrapunto.es
// Connection: keep-alive
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
// User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/28.0.1500.71 Safari/537.36
// Accept-Encoding: gzip,deflate,sdch
// Accept-Language: en-US,en;q=0.8

char *raw_packet_ethernet_ip_tcp_http_barrapunto_get = 
"\x00\x1a\xa0\x1e\x39\xfb\x5c\xf9\xdd\x54\x92\xb3\x08\x00\x45\x00"
"\x01\x73\x01\xe2\x40\x00\x80\x06\x2d\xfe\xac\x1e\x03\xbd\x5e\x17"
"\xbb\xb2\xd2\xa1\x00\x50\xa5\xa7\xb3\x8f\x60\x31\xb0\xd9\x50\x18"
"\x40\x29\xd0\x45\x00\x00\x47\x45\x54\x20\x2f\x20\x48\x54\x54\x50"
"\x2f\x31\x2e\x31\x0d\x0a\x48\x6f\x73\x74\x3a\x20\x77\x77\x77\x2e"
"\x62\x61\x72\x72\x61\x70\x75\x6e\x74\x6f\x2e\x65\x73\x0d\x0a\x43"
"\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e\x3a\x20\x6b\x65\x65\x70\x2d"
"\x61\x6c\x69\x76\x65\x0d\x0a\x41\x63\x63\x65\x70\x74\x3a\x20\x74"
"\x65\x78\x74\x2f\x68\x74\x6d\x6c\x2c\x61\x70\x70\x6c\x69\x63\x61"
"\x74\x69\x6f\x6e\x2f\x78\x68\x74\x6d\x6c\x2b\x78\x6d\x6c\x2c\x61"
"\x70\x70\x6c\x69\x63\x61\x74\x69\x6f\x6e\x2f\x78\x6d\x6c\x3b\x71"
"\x3d\x30\x2e\x39\x2c\x2a\x2f\x2a\x3b\x71\x3d\x30\x2e\x38\x0d\x0a"
"\x55\x73\x65\x72\x2d\x41\x67\x65\x6e\x74\x3a\x20\x4d\x6f\x7a\x69"
"\x6c\x6c\x61\x2f\x35\x2e\x30\x20\x28\x57\x69\x6e\x64\x6f\x77\x73"
"\x20\x4e\x54\x20\x36\x2e\x31\x3b\x20\x57\x4f\x57\x36\x34\x29\x20"
"\x41\x70\x70\x6c\x65\x57\x65\x62\x4b\x69\x74\x2f\x35\x33\x37\x2e"
"\x33\x36\x20\x28\x4b\x48\x54\x4d\x4c\x2c\x20\x6c\x69\x6b\x65\x20"
"\x47\x65\x63\x6b\x6f\x29\x20\x43\x68\x72\x6f\x6d\x65\x2f\x32\x38"
"\x2e\x30\x2e\x31\x35\x30\x30\x2e\x37\x31\x20\x53\x61\x66\x61\x72"
"\x69\x2f\x35\x33\x37\x2e\x33\x36\x0d\x0a\x41\x63\x63\x65\x70\x74"
"\x2d\x45\x6e\x63\x6f\x64\x69\x6e\x67\x3a\x20\x67\x7a\x69\x70\x2c"
"\x64\x65\x66\x6c\x61\x74\x65\x2c\x73\x64\x63\x68\x0d\x0a\x41\x63"
"\x63\x65\x70\x74\x2d\x4c\x61\x6e\x67\x75\x61\x67\x65\x3a\x20\x65"
"\x6e\x2d\x55\x53\x2c\x65\x6e\x3b\x71\x3d\x30\x2e\x38\x0d\x0a\x0d"
"\x0a";

int raw_packet_ethernet_ip_tcp_http_barrapunto_get_length = 385;



char *raw_packet_ethernet_ip_udp_gprs_ip_udp_dns_request =
"\x00\x05\x47\x02\xcd\xe3\x00\x21\x1c\x58\xb8\xaa\x08\x00\x45\x48"
"\x00\x62\x00\x00\x00\x00\xf8\x11\x30\xf9\xd4\x44\xaa\xb0\xd4\x44"
"\x3e\x10\x08\x68\x08\x68\x00\x4e\x8f\x81\x30\xff\x00\x3e\x58\x6a"
"\xee\x2d\x45\x00\x00\x3e\x42\x6b\x00\x00\x80\x11\x4e\x61\x1c\x66"
"\x06\x24\xd4\xbe\xb2\x9a\xfd\x8b\x00\x35\x00\x2a\x33\xe4\x2d\xf7"
"\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x03\x77\x77\x77\x08\x66"
"\x61\x63\x65\x62\x6f\x6f\x6b\x03\x63\x6f\x6d\x00\x00\x1c\x00\x01";

int raw_packet_ethernet_ip_udp_gprs_ip_udp_dns_request_length = 112;

// GTP v1 packet

char *raw_packet_ethernet_ip_udp_gtpv1_ip_icmp_echo = 

"\x00\x05\x47\x02\xcd\xe3\x00\x05\x47\x02\x88\x4a\x08\x00\x45\x00"
"\x00\x48\x00\x00\x00\x00\xfe\x11\x97\x8f\xd0\x40\x1e\x7c\xa4\x14"
"\x3e\x1e\x08\x68\x08\x68\x00\x34\xc6\x17\x30\xff\x00\x24\xfb\x49"
"\xd7\x1b\x45\x00\x00\x24\x00\x04\x00\x00\xff\x01\x36\x17\x0c\x13"
"\x7e\xe2\x1e\xe1\x5c\x01\x08\x00\x54\x32\x12\x34\x00\x04\x61\x62"
"\x63\x64\x65\x66\x67\x68";

int raw_packet_ethernet_ip_udp_gtpv1_ip_icmp_echo_length = 86;


#endif
