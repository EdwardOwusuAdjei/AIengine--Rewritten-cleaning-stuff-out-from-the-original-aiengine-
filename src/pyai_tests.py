#!/usr/bin/env python
#
# AIEngine.
#
# Copyright (C) 2013-2015  Luis Campo Giralte
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
# Boston, MA  02110-1301, USA.
#
# Written by Luis Campo Giralte <luis.camp0.2009@gmail.com> 
#
""" Unit tests for the pyaiengine python wrapper """
import os
import signal
import sys
import pyaiengine
import unittest

""" For python compatibility """
try:
    xrange
except NameError:
    xrange = range

class databaseTestAdaptor(pyaiengine.DatabaseAdaptor):
    def __init__(self):
        self.__total_inserts = 0
        self.__total_updates = 0
        self.__total_removes = 0

    def update(self,key,data):
        self.__total_updates = self.__total_updates + 1 
    
    def insert(self,key):
        self.__total_inserts = self.__total_inserts + 1
 
    def remove(self,key):
        self.__total_removes = self.__total_removes + 1

    def getInserts(self):
        return self.__total_inserts

    def getUpdates(self):
        return self.__total_updates

    def getRemoves(self):
        return self.__total_removes

class StackLanTests(unittest.TestCase):

    def setUp(self):
        self.s = pyaiengine.StackLan()
        self.dis = pyaiengine.PacketDispatcher() 
        self.dis.stack = self.s
        self.s.tcpflows = 2048
        self.s.udpflows = 1024
        self.called_callback = 0 
        self.ip_called_callback = 0 

    def tearDown(self):
        del self.s
        del self.dis

    def test1(self):
        """ Create a regex for netbios and detect """
        self.s.linklayertag = "vlan"

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("netbios","CACACACA")
        rm.addRegex(r)
        self.s.udpregexmanager = rm

        self.dis.open("../pcapfiles/flow_vlan_netbios.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(r.matchs, 1)
        self.assertEqual(self.s.udpregexmanager, rm)
        self.assertEqual(self.s.linklayertag,"vlan")

    def test2(self):
        """ Verify that None is working on the udpregexmanager """
        self.s.linklayertag = "vlan"

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("netbios","CACACACA")
        rm.addRegex(r)
        self.s.udpregexmanager = rm
        self.s.udpregexmanager = None

        self.dis.open("../pcapfiles/flow_vlan_netbios.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(r.matchs, 0)
        self.assertEqual(self.s.udpregexmanager, None)

    def test3(self):
        """ Create a regex for netbios with callback """
        def callback(flow):
            self.called_callback += 1 
            self.assertEqual(flow.regex.matchs,1)
            self.assertEqual(flow.regex.name, "netbios")
    
        self.s.linklayertag = "vlan"

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("netbios","CACACACA")
        r.callback = callback
        rm.addRegex(r)
        self.s.udpregexmanager = rm

        self.dis.open("../pcapfiles/flow_vlan_netbios.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(r.matchs, 1)
        self.assertEqual(self.called_callback, 1)

    def test4(self):
        """ Verify DNS and HTTP traffic """

        self.dis.open("../pcapfiles/accessgoogle.pcap");
        self.dis.run();
        self.dis.close();

        ft = self.s.tcpflowmanager 
        fu = self.s.udpflowmanager

        self.assertEqual(len(ft), 1)
        self.assertEqual(len(fu), 1)

        for flow in self.s.udpflowmanager:
    	    udp_flow = flow
    	    break

        self.assertEqual(str(udp_flow.dnsinfo.domainname),"www.google.com")	

        """ Verify the properties of the flows """
        self.assertEqual(str(udp_flow.srcip),"192.168.1.13")
        self.assertEqual(str(udp_flow.dstip),"89.101.160.5")
        self.assertEqual(int(udp_flow.srcport),54737)
        self.assertEqual(int(udp_flow.dstport),53)

        for flow in ft:
    	    http_flow = flow
    	    break

        """ Read only attributes """
        self.assertEqual(http_flow.packetslayer7, 4)
        self.assertEqual(http_flow.packets,10)
        self.assertEqual(http_flow.bytes, 1826)
        self.assertEqual(http_flow.havetag, False)

        self.assertEqual(str(http_flow.httpinfo.hostname),"www.google.com")

    def test5(self):
        """ Verify SSL traffic """

        self.dis.open("../pcapfiles/sslflow.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(len(self.s.tcpflowmanager), 1)

        for flow in self.s.tcpflowmanager:
            f = flow
            break

        self.assertEqual(str(f.sslinfo.servername),"0.drive.google.com")

    def test6(self):
        """ Verify SSL traffic with domain callback"""
        
        def domain_callback(flow):
            self.called_callback += 1 

        d = pyaiengine.DomainName("Google Drive Cert",".drive.google.com")
        d.callback = domain_callback

        dm = pyaiengine.DomainNameManager()
        dm.addDomainName(d)

        self.s.setDomainNameManager(dm,"SSLProtocol")

        self.dis.open("../pcapfiles/sslflow.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(len(dm), 1)
        self.assertEqual(d.matchs , 1)
        self.assertEqual(self.called_callback, 1)

        """ check also the integrity of the ssl cache and counters """
        cc = self.s.getCounters("SSLProtocol")
        ca = self.s.getCache("SSLProtocol")
        self.assertEqual(len(ca),1)
        self.assertEqual(cc['server hellos'], 1)

    def test7(self):
        """ Verify SSL traffic with domain callback and IPset"""

        def ipset_callback(flow):
            self.ip_called_callback += 1

        def domain_callback(flow):
            self.called_callback += 1 

        ip = pyaiengine.IPSet("Specific IP address")
        ip.addIPAddress("74.125.24.189")
        ip.callback = ipset_callback

        ipm = pyaiengine.IPSetManager()
        ipm.addIPSet(ip)

        d = pyaiengine.DomainName("Google All",".google.com")
        d.callback = domain_callback

        dm = pyaiengine.DomainNameManager()
        dm.addDomainName(d)

        self.s.tcpipsetmanager = ipm
        self.s.setDomainNameManager(dm,"SSLProtocol")

        self.dis.open("../pcapfiles/sslflow.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(d.matchs, 1)
        self.assertEqual(self.called_callback,1)
        self.assertEqual(self.ip_called_callback,1)

    def test8(self):
        """ Attach a database to the engine """

        db = databaseTestAdaptor()

        self.s.setTCPDatabaseAdaptor(db,16)

        self.dis.open("../pcapfiles/sslflow.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(db.getInserts(), 1)
        self.assertEqual(db.getUpdates(), 5)
        self.assertEqual(db.getRemoves(), 0)

    def test9(self):
        """ Attach a database to the engine """

        db = databaseTestAdaptor()

        self.s.linklayertag  = "vlan"
        self.s.setUDPDatabaseAdaptor(db,16)

        self.dis.open("../pcapfiles/flow_vlan_netbios.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(db.getInserts(), 1)
        self.assertEqual(db.getUpdates(), 1)
        self.assertEqual(db.getRemoves(), 0)


    def test10(self):
        """ Attach a database to the engine and domain name"""

        def domain_callback(flow):
            self.called_callback += 1 
            self.assertEqual(str(flow.sslinfo.servername),"0.drive.google.com")
            self.assertEqual(flow.l7protocolname,"SSLProtocol")

        d = pyaiengine.DomainName("Google All",".google.com")

        dm = pyaiengine.DomainNameManager()
        d.callback = domain_callback
        dm.addDomainName(d)

        self.s.setDomainNameManager(dm,"SSLProtocol")

        db = databaseTestAdaptor()

        self.s.setTCPDatabaseAdaptor(db,16)

        self.dis.open("../pcapfiles/sslflow.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(db.getInserts(), 1)
        self.assertEqual(db.getUpdates(), 5)
        self.assertEqual(db.getRemoves(), 0)
        self.assertEqual(d.matchs,  1)
        self.assertEqual(self.called_callback, 1)

    def test11(self):
        """ Verify iterators of the RegexManager """

        rl = [ pyaiengine.Regex("expression %d" % x, "some regex %d" % x) for x in xrange(0,5) ]

        rm = pyaiengine.RegexManager()

        [rm.addRegex(r) for r in rl] 
 
        self.assertEqual(self.s.tcpregexmanager, None) 
      
        self.s.tcpregexmanager = rm 
        self.s.enableNIDSEngine(True)	

        self.dis.open("../pcapfiles/sslflow.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(len(rm), 5)
    
        self.assertEqual(rm, self.s.tcpregexmanager)
        for r in rl:
    	    self.assertEqual(r.matchs, 0)

    def test12(self):
        """ Verify the IPBloomSet class """

        have_bloom = False
        try:
            from pyaiengine import IPBloomSet 
            have_bloom = True
        except ImportError:
            pass
  
        if (have_bloom): # execute the test
            def ipset_callback(flow):
                self.ip_called_callback += 1

            ip = pyaiengine.IPBloomSet("Specific IP address")
            ip = IPBloomSet("Specific IP address")
            ip.addIPAddress("74.125.24.189")
            ip.callback = ipset_callback

            ipm = pyaiengine.IPSetManager()
            ipm.addIPSet(ip)

            self.s.tcpipsetmanager = ipm

            self.dis.open("../pcapfiles/sslflow.pcap");
            self.dis.run();
            self.dis.close();

            self.assertEqual(self.ip_called_callback,1)

    def test13(self):
        """ Verify all the URIs of an HTTP flow """

        def domain_callback(flow):
            urls = ("/css/global.css?v=20121120a","/js/jquery.hoverIntent.js","/js/ecom/ecomPlacement.js","/js/scrolldock/scrolldock.css?v=20121120a",
                "/images_blogs/gadgetlab/2013/07/MG_9640edit-200x100.jpg","/images_blogs/underwire/2013/08/Back-In-Time-200x100.jpg",
                "/images_blogs/thisdayintech/2013/03/set.jpg","/js/scrolldock/i/sub_righttab.gif","/images/global_header/new/Marriott_217x109.jpg",
                "/images/global_header/subscribe/gh_flyout_failsafe.jpg","/images/global_header/new/the-connective.jpg","/images/covers/120x164.jpg",
                "/images/subscribe/xrail_headline.gif","/images_blogs/gadgetlab/2013/08/bb10-bg.jpg","/images_blogs/autopia/2013/08/rescuer_cam06_110830-200x100.jpg",
                "/images_blogs/wiredscience/2013/08/earth-ring-200x100.jpg","/images_blogs/underwire/2013/08/breaking-bad-small-200x100.png",
                "/insights/wp-content/uploads/2013/08/dotcombubble_660-200x100.jpg","/geekdad/wp-content/uploads/2013/03/wreck-it-ralph-title1-200x100.png",
                "/wiredenterprise/wp-content/uploads/2013/08/apple-logo-pixels-200x100.jpg","/images_blogs/threatlevel/2013/08/drone-w.jpg",
                "/images_blogs/rawfile/2013/08/CirculationDesk-200x100.jpg","/images_blogs/magazine/2013/07/theoptimist_wired-200x100.jpg",
                "/images_blogs/underwire/2013/08/Back-In-Time-w.jpg","/design/wp-content/uploads/2013/08/dyson-w.jpg",
                "/images_blogs/threatlevel/2013/08/aaron_swartz-w.jpg","/images_blogs/threatlevel/2013/08/aaron_swartz-w.jpg",
                "/images_blogs/wiredscience/2013/08/NegativelyRefracting-w.jpg","/images_blogs/wiredscience/2013/08/bee-w.jpg",
                "/gadgetlab/2013/08/blackberry-failures/","/gadgetlab/wp-content/themes/wired-global/style.css?ver=20121114",
                "/css/global.css?ver=20121114","/js/cn-fe-common/jquery-1.7.2.min.js?ver=1.7.2","/js/cn.minified.js?ver=20121114",
                "/js/videos/MobileCompatibility.js?ver=20121114","/images_blogs/gadgetlab/2013/06/internets.png",
                "/gadgetlab/wp-content/themes/wired-responsive/i/design-sprite.png","/images_blogs/gadgetlab/2013/08/Blackberry8820.jpg",
                "/images_blogs/gadgetlab/2013/08/vsapple-60x60.jpg","/images_blogs/gadgetlab/2013/08/AP090714043057-60x60.jpg"
            )
            self.called_callback += 1

            sw = False
            for url in urls:
                if (str(flow.httpinfo.uri) == url):
                    sw = True

            self.assertEqual(sw,True)
            self.assertEqual(str(flow.httpinfo.hostname),"www.wired.com")
            self.assertEqual(flow.l7protocolname,"HTTPProtocol")

        d = pyaiengine.DomainName("Wired domain",".wired.com")

        dm = pyaiengine.DomainNameManager()
        d.callback = domain_callback
        dm.addDomainName(d)

        self.s.setDomainNameManager(dm,"HTTPProtocol")

        self.dis.open("../pcapfiles/two_http_flows_noending.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(self.called_callback, 1)

    def test14(self):
        """ Verify cache release functionality """

        self.s.flowstimeout = 50000000 # No timeout :D

        self.dis.open("../pcapfiles/sslflow.pcap")
        self.dis.run()
        self.dis.close()
        
        ft = self.s.tcpflowmanager

        self.assertEqual(len(ft), 1)

        for flow in ft:
            self.assertNotEqual(flow.sslinfo,None)
       
        self.dis.open("../pcapfiles/accessgoogle.pcap")
        self.dis.run()
        self.dis.close()

        fu = self.s.udpflowmanager

        self.assertEqual(len(fu), 1)

        for flow in fu:
            self.assertNotEqual(flow.dnsinfo,None)

        # release some of the caches
        self.s.releaseCache("SSLProtocol")
        
        for flow in ft:
            self.assertEqual(flow.sslinfo,None)

        # release all the caches
        self.s.releaseCaches()

        for flow in ft:
            self.assertEqual(flow.sslinfo,None)
            self.assertEqual(flow.httpinfo,None)

        for flow in fu:
            self.assertEqual(flow.dnsinfo,None)

    def test15(self):
        """ Attach a database to the engine and test timeouts on udp flows """

        db = databaseTestAdaptor()

        self.s.linklayertag = "vlan"
        self.s.setUDPDatabaseAdaptor(db,16)

        self.s.flowstimeout = 1

        self.dis.open("../pcapfiles/flow_vlan_netbios.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(db.getInserts(), 1)
        self.assertEqual(db.getUpdates(), 1)
        self.assertEqual(db.getRemoves(), 1)
        self.assertEqual(self.s.flowstimeout, 1)

    def test16(self):
        """ Verify that ban domains dont take memory """

        d = pyaiengine.DomainName("Wired domain",".wired.com")

        dm = pyaiengine.DomainNameManager()
        dm.addDomainName(d)

        self.s.setDomainNameManager(dm,"HTTPProtocol",False)

        self.dis.open("../pcapfiles/two_http_flows_noending.pcap")
        self.dis.pcapfilter = "tcp" 
        self.dis.run()
        self.dis.close()

        self.assertEqual(d.matchs, 1)

        ft = self.s.tcpflowmanager

        self.assertEqual(len(ft), 2)
        self.assertEqual(self.dis.pcapfilter, "tcp")

        # Only the first flow is the banned
        for flow in ft:
            info = flow.httpinfo
            self.assertEqual(info.hostname, '')
            self.assertEqual(info.useragent, '')
            self.assertEqual(info.uri, '')
            break

    def test17(self):
        """ Verify the ban functionatly on the fly with a callback """

        def domain_callback(flow):
            self.called_callback += 1
            
            info = flow.httpinfo
            url = info.uri

            # Some URI analsys on the first request could be done here
            if (url == "/css/global.css?v=20121120a"):
                info.banned = True

        d = pyaiengine.DomainName("Wired domain",".wired.com")

        dm = pyaiengine.DomainNameManager()
        d.callback = domain_callback
        dm.addDomainName(d)

        self.s.setDomainNameManager(dm,"HTTPProtocol")

        self.dis.open("../pcapfiles/two_http_flows_noending.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(self.called_callback, 1)

        ft = self.s.tcpflowmanager

        self.assertEqual(len(ft), 2)

        # Only the first flow is the banned and released
        for flow in self.s.tcpflowmanager:
            inf = flow.httpinfo
            self.assertNotEqual(inf, None)
            self.assertEqual(inf.uri, '')
            self.assertEqual(inf.useragent, '')
            self.assertEqual(inf.hostname, '')
            break

    def test18(self):
        """ Verify the getCounters functionatly """

        self.dis.open("../pcapfiles/two_http_flows_noending.pcap")
        self.dis.run()
        self.dis.close()

        c = self.s.getCounters("EthernetProtocol")

        self.assertEqual(c.has_key("packets"), True) 
        self.assertEqual(c.has_key("bytes"), True) 
        self.assertEqual(c["bytes"], 910064)

        c = self.s.getCounters("TCPProtocol")

        self.assertEqual(c["bytes"], 879940)
        self.assertEqual(c["packets"], 886)
        self.assertEqual(c["syns"], 2)
        self.assertEqual(c["synacks"], 2)
        self.assertEqual(c["acks"], 882)
        self.assertEqual(c["rsts"], 0)
        self.assertEqual(c["fins"], 0)

        c = self.s.getCounters("UnknownProtocol")
        self.assertEqual(len(c), 0)

    def test19(self):
        """ Verify SMTP traffic with domain callback """
        self.from_correct = False
        def domain_callback(flow):
            s = flow.smtpinfo
            if (s):
                if (str(s.mailfrom) == "gurpartap@patriots.in"):
                    self.from_correct = True
            self.called_callback += 1

        d = pyaiengine.DomainName("Some domain",".patriots.in")
        d.callback = domain_callback

        dm = pyaiengine.DomainNameManager()
        dm.addDomainName(d)

        self.s.setDomainNameManager(dm,"SMTPProtocol")

        oldstack = None

        with pyaiengine.PacketDispatcher("../pcapfiles/smtp.pcap") as pd:
            pd.stack = self.s
            pd.run();
            oldstack = pd.stack

        self.assertEqual(oldstack,self.s)

        self.assertEqual(d.matchs , 1)
        self.assertEqual(self.called_callback,1)
        self.assertEqual(self.from_correct,True)

    def test20(self):
        """ Test the chains of regex with RegexManagers """

        rlist = [ pyaiengine.Regex("expression %d" % x, "some regex %d" % x) for x in xrange(0,5) ]

        rmbase = pyaiengine.RegexManager()
        rm1 = pyaiengine.RegexManager()
        rm2 = pyaiengine.RegexManager()
        rm3 = pyaiengine.RegexManager()

        [rmbase.addRegex(r) for r in rlist]

	r1 = pyaiengine.Regex("smtp1" , "^AUTH LOGIN")
	r1.nextregexmanager = rm1
	rmbase.addRegex(r1)

	r2 = pyaiengine.Regex("smtp2" , "^NO MATCHS")
	r3 = pyaiengine.Regex("smtp3" , "^MAIL FROM")

	rm1.addRegex(r2)
	rm1.addRegex(r3)
	r3.nextregexmanager = rm2	

	r4 = pyaiengine.Regex("smtp4" , "^NO MATCHS")
	r5 = pyaiengine.Regex("smtp5" , "^DATA")
	
	rm2.addRegex(r4)
	rm2.addRegex(r5)
	r5.nextregexmanager = rm3

	r6 = pyaiengine.Regex("smtp6" , "^QUIT")
	rm3.addRegex(r6)

        self.s.tcpregexmanager = rmbase
        self.s.enableNIDSEngine(True)

        with pyaiengine.PacketDispatcher("../pcapfiles/smtp.pcap") as pd:
            pd.stack = self.s
            pd.run();

	for r in rlist:
        	self.assertEqual(r.matchs , 0)

	self.assertEqual(r1.matchs, 1)
	self.assertEqual(r2.matchs, 0)
	self.assertEqual(r3.matchs, 1)
	self.assertEqual(r4.matchs, 0)
	self.assertEqual(r5.matchs, 1)
	self.assertEqual(r6.matchs, 1)

    def test21(self):
        """ Tests the parameters of the callbacks """
        def callback1(flow):
            pass

        def callback2(flow,other):
            pass

        def callback3():
            pass

        r = pyaiengine.Regex("netbios","CACACACA")

        try: 
            r.callback = None
            self.assertTrue(False)
        except:
            self.assertTrue(True)

        try:
            r.callback = callback2
            self.assertTrue(False)
        except:
            self.assertTrue(True)

        try:
            r.callback = callback1
            self.assertTrue(True)
        except:
            self.assertTrue(False)

    def test22(self):
        """ Verify the functionatliy of the HTTPUriSets with the callbacks """

        self.uset = pyaiengine.HTTPUriSet()
        def domain_callback(flow):
            self.called_callback += 1

        def uri_callback(flow):
            self.assertEqual(self.uset.uris, 1)
            self.assertEqual(self.uset.lookups, 39)
            self.assertEqual(self.uset.lookupsin, 1)
            self.assertEqual(self.uset.lookupsout, 38)
            self.called_callback += 1

        d = pyaiengine.DomainName("Wired domain",".wired.com")

        dm = pyaiengine.DomainNameManager()
        d.callback = domain_callback
        dm.addDomainName(d)

        self.uset.addURI("/images_blogs/gadgetlab/2013/08/AP090714043057-60x60.jpg")
        # self.uset.addURI("/js/jquery.hoverIntent.js")
        self.uset.callback = uri_callback

	d.httpuriset = self.uset

        self.s.setDomainNameManager(dm,"HTTPProtocol")

        with pyaiengine.PacketDispatcher("../pcapfiles/two_http_flows_noending.pcap") as pd:
            pd.stack = self.s
            pd.run();

        self.assertEqual(d.httpuriset, self.uset)
        self.assertEqual(self.uset.uris, 1)
        self.assertEqual(self.uset.lookups, 39)
        self.assertEqual(self.uset.lookupsin, 1)
        self.assertEqual(self.uset.lookupsout, 38)

        self.assertEqual(self.called_callback,2)

    def test23(self):
        """ Verify the functionatliy of the HTTPUriSets with the callbacks """

        self.uset = pyaiengine.HTTPUriSet()
        def domain_callback(flow):
            self.called_callback += 1

        def uri_callback(flow):
            self.assertEqual(self.uset.uris, 1)
            self.assertEqual(self.uset.lookups, 4)
            self.assertEqual(self.uset.lookupsin, 1)
            self.assertEqual(self.uset.lookupsout, 3)
            self.called_callback += 1

        d = pyaiengine.DomainName("Wired domain",".wired.com")

        dm = pyaiengine.DomainNameManager()
        d.callback = domain_callback
        dm.addDomainName(d)

	# This uri is the thrid of the wired.com flow
        self.uset.addURI("/js/scrolldock/scrolldock.css?v=20121120a")
        self.uset.callback = uri_callback

        d.httpuriset = self.uset

        self.s.setDomainNameManager(dm,"HTTPProtocol")

        with pyaiengine.PacketDispatcher("../pcapfiles/two_http_flows_noending.pcap") as pd:
            pd.stack = self.s
            pd.run();

        self.assertEqual(self.uset.uris, 1)
        self.assertEqual(self.uset.lookups, 39)
        self.assertEqual(self.uset.lookupsin, 1)
        self.assertEqual(self.uset.lookupsout, 38)
        self.assertEqual(self.called_callback,2)

    def test24(self):
        """ Verify the property of the PacketDispatcher.stack """

        p = pyaiengine.PacketDispatcher()

        self.assertEqual(p.stack, None)
        
        # p.stack = p 
        self.dis.stack = None

        self.assertEqual(self.dis.stack, None)

    def test25(self):
        """ Verify the functionatliy of the SSDP Protocol """
   
        with pyaiengine.PacketDispatcher("../pcapfiles/ssdp_flow.pcap") as pd:
            pd.stack = self.s
            pd.run();

        fu = self.s.udpflowmanager
        for flow in fu:
            s = flow.ssdpinfo
            if (s):
                self.assertEqual(s.uri,"*")
                self.assertEqual(s.hostname,"239.255.255.250:1900")
            else:
                self.assertFalse(False) 

    def test26(self):
        """ Verify the functionatliy of the RegexManager on the HTTP Protocol for analise
            inside the l7 payload of HTTP """

        def callback_domain(flow):
            self.called_callback += 1
            pass

        def callback_regex(flow):
            self.called_callback += 1
            self.assertEqual(flow.packets, 11)
            self.assertEqual(flow.packetslayer7, 4)
 
        d = pyaiengine.DomainName("Wired domain",".wired.com")

        rm = pyaiengine.RegexManager()
        r1 = pyaiengine.Regex("Regex for analysing the content of HTTP",b"^\x1f\x8b\x08\x00\x00\x00\x00.*$")
        r2 = pyaiengine.Regex("Regex for analysing the content of HTTP",b"^.{3}\xcd\x9c\xc0\x0a\x34.*$")
        r3 = pyaiengine.Regex("Regex for analysing the content of HTTP",b"^.*\x44\x75\x57\x0c\x22\x7b\xa7\x6d$")

	r2.nextregex = r3
	r1.nextregex = r2
        rm.addRegex(r1)
        r3.callback = callback_regex

        """ So the flows from wired.com will be analise the regexmanager attached """
        d.regexmanager = rm

        dm = pyaiengine.DomainNameManager()
        d.callback = callback_domain
        dm.addDomainName(d)

        self.s.setDomainNameManager(dm,"HTTPProtocol")

        with pyaiengine.PacketDispatcher("../pcapfiles/two_http_flows_noending.pcap") as pd:
            pd.stack = self.s
            pd.run();

        self.assertEqual(self.called_callback, 2)
        self.assertEqual(r1.matchs, 1)
        self.assertEqual(r2.matchs, 1)
        self.assertEqual(r3.matchs, 1)
        self.assertEqual(d.matchs, 1)
    
    def test27(self):
        """ Verify the correctness of the HTTP Protocol """ 

        """ The filter tcp and port 55354 will filter just one HTTP flow
            that contains exactly 39 requests and 38 responses """
        with pyaiengine.PacketDispatcher("../pcapfiles/two_http_flows_noending.pcap") as pd:
            pd.pcapfilter = "tcp and port 55354"
            pd.stack = self.s
            pd.run();

        c = self.s.getCounters("HTTPProtocol")
        self.assertEqual(c["requests"], 39)
        self.assertEqual(c["responses"], 38)

    def test28(self):
        """ Verify the correctness of the HTTP Protocol """

        """ The filter tcp and port 49503 will filter just one HTTP flow
            that contains exactly 39 requests and 38 responses """
        with pyaiengine.PacketDispatcher("../pcapfiles/two_http_flows_noending.pcap") as pd:
            pd.pcapfilter = "tcp and port 49503"
            pd.stack = self.s
            pd.run();

        c = self.s.getCounters("HTTPProtocol")
        self.assertEqual(c["requests"], 3)
        self.assertEqual(c["responses"], 3)


   
class StackLanIPv6Tests(unittest.TestCase):

    def setUp(self):
        self.s = pyaiengine.StackLanIPv6()
        self.dis = pyaiengine.PacketDispatcher()
        self.dis.stack = self.s
        self.s.tcpflows = 2048
        self.s.udpflows = 1024
        self.called_callback = 0

    def tearDown(self):
        del self.s
        del self.dis

    def test1(self):
        """ Create a regex for a generic exploit """

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("generic exploit",b"\x90\x90\x90\x90\x90\x90\x90")
        rm.addRegex(r)
        self.s.tcpregexmanager = rm

        self.dis.open("../pcapfiles/generic_exploit_ipv6_defcon20.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r.matchs, 1)

    def test2(self):
        """ Create a regex for a generic exploit and a IPSet """
        def ipset_callback(flow):
            self.called_callback += 1 

        ipset = pyaiengine.IPSet("IPv6 generic set")
        ipset.addIPAddress("dc20:c7f:2012:11::2")
        ipset.addIPAddress("dc20:c7f:2012:11::1")
        ipset.callback = ipset_callback
        im = pyaiengine.IPSetManager()

        im.addIPSet(ipset)
        self.s.tcpipsetmanager = im

        rm = pyaiengine.RegexManager()
        r1 = pyaiengine.Regex("generic exploit","\x90\x90\x90\x90\x90\x90\x90")
        rm.addRegex(r1)
        r2 = pyaiengine.Regex("other exploit","(this can not match)")
        rm.addRegex(r2)
        self.s.tcpregexmanager = rm

        self.dis.open("../pcapfiles/generic_exploit_ipv6_defcon20.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r1.matchs, 1)
        self.assertEqual(r2.matchs, 0)
        self.assertEqual(self.called_callback , 1)

    def test3(self):
        """ Create a regex for a generic exploit and a IPSet with no matching"""
        def ipset_callback(flow):
            self.called_callback += 1

        ipset = pyaiengine.IPSet()
        ipset.addIPAddress("dc20:c7f:2012:11::22")
        ipset.addIPAddress("dc20:c7f:2012:11::1")
        ipset.callback = ipset_callback
        im = pyaiengine.IPSetManager()

        im.addIPSet(ipset)
        self.s.tcpipsetmanager = im

        rm = pyaiengine.RegexManager()
        r1 = pyaiengine.Regex("generic exploit","\xaa\xbb\xcc\xdd\x90\x90\x90")
        rm.addRegex(r1)
        r2 = pyaiengine.Regex("other exploit","(this can not match)")
        rm.addRegex(r2)
        self.s.tcpregexmanager = rm

        self.dis.open("../pcapfiles/generic_exploit_ipv6_defcon20.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r1.matchs, 0)
        self.assertEqual(r2.matchs, 0)
        self.assertEqual(self.called_callback , 0)

    def test4(self):
        """ Attach a database to the engine for TCP traffic """

        db = databaseTestAdaptor()
        
        self.s.setTCPDatabaseAdaptor(db,16)

        self.dis.open("../pcapfiles/generic_exploit_ipv6_defcon20.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(db.getInserts(), 1)
        self.assertEqual(db.getUpdates(), 5)
        self.assertEqual(db.getRemoves(), 1)

    def test5(self):
        """ Attach a database to the engine for UDP traffic """

        db_udp = databaseTestAdaptor()
        db_tcp = databaseTestAdaptor()

        self.s.setUDPDatabaseAdaptor(db_udp,16)
        self.s.setTCPDatabaseAdaptor(db_tcp)

        self.dis.open("../pcapfiles/ipv6_google_dns.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(db_udp.getInserts(), 1)
        self.assertEqual(db_udp.getUpdates(), 1)
        self.assertEqual(db_udp.getRemoves(), 0)

        self.assertEqual(db_tcp.getInserts(), 0)
        self.assertEqual(db_tcp.getUpdates(), 0)
        self.assertEqual(db_tcp.getRemoves(), 0)

    def test6(self):
        """ Several IPSets with no matching"""
        def ipset_callback(flow):
            self.called_callback += 1

        ipset1 = pyaiengine.IPSet("IPSet 1")
        ipset2 = pyaiengine.IPSet("IPSet 2")
        ipset3 = pyaiengine.IPSet("IPSet 3")
        ipset3.addIPAddress("dc20:c7f:2012:11::2")
        ipset2.addIPAddress("dcaa:c7f:2012:11::22")
        ipset1.addIPAddress("dcbb:c7f:2012:11::22")
        im = pyaiengine.IPSetManager()

        im.addIPSet(ipset1)
        im.addIPSet(ipset2)
        im.addIPSet(ipset3)

        self.s.tcpipsetmanager = im

        self.dis.open("../pcapfiles/generic_exploit_ipv6_defcon20.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(len(im), 3)
        self.assertEqual(self.called_callback , 0)
        self.assertEqual(self.s.tcpipsetmanager , im)

    def test7(self):
        """ Extract IPv6 address from a DomainName matched """
        def dns_callback(flow):
            for ip in flow.dnsinfo:
                if (ip == "2607:f8b0:4001:c05::6a"):
                    self.called_callback += 1

        d = pyaiengine.DomainName("Google test",".google.com")
        d.callback = dns_callback

        dm = pyaiengine.DomainNameManager()
        dm.addDomainName(d)

        self.s.setDomainNameManager(dm,"DNSProtocol")

        self.dis.open("../pcapfiles/ipv6_google_dns.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(self.called_callback , 1)

    def test8(self):
        """ Test the functionality of make graphs of regex, for complex detecctions """ 

        rmbase = pyaiengine.RegexManager()
        rm2 = pyaiengine.RegexManager()
        r1 = pyaiengine.Regex("r1",b"^(No hacker should visit Las Vegas).*$")
      
        rmbase.addRegex(r1)

        r1.nextregexmanager = rm2 

        r2 = pyaiengine.Regex("r2",b"(this can not match)")
        r3 = pyaiengine.Regex("r3",b"^\x90\x90\x90\x90.*$")
        rm2.addRegex(r2)
        rm2.addRegex(r3)

        self.s.tcpregexmanager = rmbase

        with pyaiengine.PacketDispatcher("../pcapfiles/generic_exploit_ipv6_defcon20.pcap") as pd:
            pd.stack = self.s
            pd.run()

        self.assertEqual(r1.matchs, 1)
        self.assertEqual(r2.matchs, 0)
        self.assertEqual(r3.matchs, 1)

    def test9(self):
        """ Another test for the functionality of make graphs of regex, for complex detecctions """

        rm1 = pyaiengine.RegexManager()
        rm2 = pyaiengine.RegexManager()
        rm3 = pyaiengine.RegexManager()
        r1 = pyaiengine.Regex("r1",b"^(No hacker should visit Las Vegas).*$")

        r1.nextregexmanager = rm2
        rm1.addRegex(r1)

        r2 = pyaiengine.Regex("r2",b"(this can not match)")
        r3 = pyaiengine.Regex("r3",b"^\x90\x90\x90\x90.*$")
        rm2.addRegex(r2)
        rm2.addRegex(r3)

	r3.nextregexmanager = rm3

	r4 = pyaiengine.Regex("r4",b"^Upgrade.*$")
	r5 = pyaiengine.Regex("r5",b"(this can not match)")

	rm3.addRegex(r4)
	rm3.addRegex(r5)

        self.s.tcpregexmanager = rm1

        oldstack = None

        with pyaiengine.PacketDispatcher("../pcapfiles/generic_exploit_ipv6_defcon20.pcap") as pd:
            pd.stack = self.s
            pd.run()
            oldstack = self.s

        self.assertEqual(self.s, oldstack)

        self.assertEqual(r1.matchs, 1)
        self.assertEqual(r2.matchs, 0)
        self.assertEqual(r3.matchs, 1)
        self.assertEqual(r4.matchs, 1)

    def test10(self):
        """ Verify the functionality of the getCache method """

        with pyaiengine.PacketDispatcher("../pcapfiles/ipv6_google_dns.pcap") as pd:
            pd.stack = self.s
            pd.run()

        d = self.s.getCache("DNSProtocol")
        self.assertEqual(len(self.s.getCache("DNSProtocol")),1)
        self.assertEqual(len(self.s.getCache("DNSProtocolNoExists")),0)
        self.s.releaseCache("DNSProtocol")
        self.assertEqual(len(self.s.getCache("DNSProtocol")),0)
        self.assertEqual(len(self.s.getCache("HTTPProtocol")),0)
        self.assertEqual(len(self.s.getCache("SSLProtocol")),0)

    def test11(self):
        """ Verify the correctness of the HTTP Protocol on IPv6 """

        with pyaiengine.PacketDispatcher("../pcapfiles/http_over_ipv6.pcap") as pd:
            pd.stack = self.s
            pd.run();

        c = self.s.getCounters("HTTPProtocol")
        self.assertEqual(c["requests"], 11)
        self.assertEqual(c["responses"], 11)

    def test12(self):
        """ Verify the functionatliy of the RegexManager on the HTTP Protocol for analise
            inside the l7 payload of HTTP on IPv6 traffic """

        def callback_domain(flow):
            self.called_callback += 1

        def callback_regex(flow):
            self.called_callback += 1
            self.assertEqual(flow.regex.name,"Regex for analysing the content of HTTP")
            self.assertEqual(flow.httpinfo.hostname,"media.us.listen.com")

        d = pyaiengine.DomainName("Music domain",".us.listen.com")

        rm = pyaiengine.RegexManager()
        r1 = pyaiengine.Regex("Regex for analysing the content of HTTP",b"^\x89\x50\x4e\x47\x0d\x0a\x1a\x0a.*$")

        rm.addRegex(r1)
        r1.callback = callback_regex

        """ So the flows from listen.com will be analise the regexmanager attached """
        d.regexmanager = rm

        dm = pyaiengine.DomainNameManager()
        d.callback = callback_domain
        dm.addDomainName(d)

        self.s.setDomainNameManager(dm,"HTTPProtocol")

        with pyaiengine.PacketDispatcher("../pcapfiles/http_over_ipv6.pcap") as pd:
            pd.stack = self.s
            pd.run();

        self.assertEqual(self.called_callback, 2)
        self.assertEqual(r1.matchs, 1)
        self.assertEqual(d.matchs, 1)
 
class StackLanLearningTests(unittest.TestCase):

    def setUp(self):
        self.s = pyaiengine.StackLan()
        self.dis = pyaiengine.PacketDispatcher()
        self.dis.stack = self.s
        self.s.tcpflows = 2048
        self.s.udpflows = 1024
        self.f = pyaiengine.FrequencyGroup()

    def tearDown(self):
        del self.s
        del self.dis
        del self.f

    def test1(self):

        self.f.reset()
        self.s.enableFrequencyEngine(True)

        self.dis.open("../pcapfiles/two_http_flows_noending.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(self.f.getTotalProcessFlows(), 0)
        self.assertEqual(self.f.getTotalComputedFrequencies(), 0)

        """ Add the TCP Flows of the FlowManager on the FrequencyEngine """
        self.f.addFlowsByDestinationPort(self.s.tcpflowmanager)
        self.f.compute()
    
        self.assertEqual(self.f.getTotalProcessFlows(), 2)
        self.assertEqual(self.f.getTotalComputedFrequencies(), 1)

    def test2(self):
        
        self.f.reset()
        self.s.enableFrequencyEngine(True)
        
        self.dis.open("../pcapfiles/tor_4flows.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(self.f.getTotalProcessFlows(), 0)
        self.assertEqual(self.f.getTotalComputedFrequencies(), 0)

        """ Add the TCP Flows of the FlowManager on the FrequencyEngine """
        self.f.addFlowsByDestinationPort(self.s.tcpflowmanager)
        self.f.compute()

        self.assertEqual(len(self.f.getReferenceFlowsByKey("80")), 4)
        self.assertEqual(len(self.f.getReferenceFlows()), 4)
        self.assertEqual(len(self.f.getReferenceFlowsByKey("8080")), 0)
        self.assertEqual(self.f.getTotalProcessFlows(), 4)
        self.assertEqual(self.f.getTotalComputedFrequencies(), 1)

    def test3(self):
        """ Integrate with the learner to generate a regex """
        learn = pyaiengine.LearnerEngine()

        self.f.reset()
        self.s.enableFrequencyEngine(True)
        
        self.dis.open("../pcapfiles/tor_4flows.pcap")
        self.dis.run()
        self.dis.close()

        """ Add the TCP Flows of the FlowManager on the FrequencyEngine """
        self.f.addFlowsByDestinationPort(self.s.tcpflowmanager)
        self.f.compute()

        flow_list = self.f.getReferenceFlows()
        self.assertEqual(self.f.getTotalComputedFrequencies(), 1)
        learn.agregateFlows(flow_list)
        learn.compute()

        self.assertTrue(learn.flowsprocess,4)
 
        """ Get the generated regex and compile with the regex module """
        try:
            rc = re.compile(learn.regex)		
            self.assertTrue(True)	
        except:
            self.assertFalse(False)	
       

class StackVirtualTests(unittest.TestCase):

    def setUp(self):
        self.s = pyaiengine.StackVirtual()
        self.dis = pyaiengine.PacketDispatcher()
        self.dis.stack = self.s
        self.s.tcpflows = 2048
        self.s.udpflows = 1024
        self.called_callback = 0

    def tearDown(self):
        del self.s
        del self.dis

    def test1(self):
        """ Create a regex for a detect the flow on a virtual network """

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("Bin directory","^bin$")
        rm.addRegex(r)
        self.s.tcpregexmanager = rm

        self.dis.open("../pcapfiles/vxlan_ftp.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r.matchs, 1)

    def test2(self):
        """ Create a regex for a detect the flow on a virtual network on the GRE side """

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("Bin directory",b"^SSH-2.0.*$")
        rm.addRegex(r)
        self.s.tcpregexmanager = rm

        self.dis.open("../pcapfiles/gre_ssh.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r.matchs, 1)

        self.assertEqual(len(self.s.tcpflowmanager), 1)
        self.assertEqual(len(self.s.udpflowmanager), 0)


    def test3(self):
        """ Inject two pcapfiles with gre and vxlan traffic and verify regex """

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("SSH activity",b"^SSH-2.0.*$")
        rm.addRegex(r)
        self.s.tcpregexmanager = rm

        self.s.enableNIDSEngine(True)	

        # The first packet of the pcapfile is from 18 sep 2014
        self.dis.open("../pcapfiles/vxlan_ftp.pcap")
        self.dis.run()
        self.dis.close()

        """ This FlowManagers points to the virtualize layer """
        ft = self.s.tcpflowmanager
        fu = self.s.udpflowmanager

        self.assertEqual(ft.flows , 1)
        self.assertEqual(ft.processflows , 1)
        self.assertEqual(ft.timeoutflows , 0)

        self.assertEqual(r.matchs, 0)
        self.assertEqual(len(self.s.tcpflowmanager), 1)
        self.assertEqual(len(self.s.udpflowmanager), 0)

        self.s.flowstimeout = (60 * 60 * 24)

        # The first packet of the pcapfile is from 19 sep 2014
        self.dis.open("../pcapfiles/gre_ssh.pcap")
        self.dis.run()
        self.dis.close()
      
        self.assertEqual(ft.flows , 2)
        self.assertEqual(ft.processflows , 2)
        self.assertEqual(ft.timeoutflows , 0)

        self.assertEqual(r.matchs, 1)
        self.assertEqual(len(ft), 2)
        self.assertEqual(len(fu), 0)

    def test4(self):
        """ Test the extraction of the tag from the flow when matches """

        def virt_callback(flow):
            if ((flow.havetag == True)and(flow.tag == 1)):
                self.called_callback += 1

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("Bin directory",b"^bin$")
        r.callback = virt_callback
        rm.addRegex(r)
        self.s.tcpregexmanager = rm

        self.s.enableNIDSEngine(True)

        self.dis.open("../pcapfiles/vxlan_ftp.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r.callback, virt_callback)
        self.assertEqual(r.matchs, 1)
        self.assertEqual(self.called_callback,1)

class StackOpenFlowTests(unittest.TestCase):

    def setUp(self):
        self.s = pyaiengine.StackOpenFlow()
        self.dis = pyaiengine.PacketDispatcher()
        self.dis.stack = self.s
        self.s.tcpflows = 2048
        self.s.udpflows = 1024
        self.called_callback = 0

    def tearDown(self):
        del self.s
        del self.dis

    def test1(self):
        """ Create a regex for a detect the flow on a openflow network """

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("Bin directory",b"^\x26\x01")
        rm.addRegex(r)
        self.s.tcpregexmanager = rm

        self.dis.open("../pcapfiles/openflow.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r.matchs, 1)

    def test2(self):
        """ Test the with statement of the PacketDispatcher """

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("Bin directory",b"^\x26\x01")
        rm.addRegex(r)
        self.s.tcpregexmanager = rm

        with pyaiengine.PacketDispatcher("../pcapfiles/openflow.pcap") as pd:
            pd.stack = self.s
            pd.run()

        self.assertEqual(r.matchs, 1)

if __name__ == '__main__':

    unittest.main()

    sys.exit(0)

