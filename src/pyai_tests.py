#!/usr/bin/env python
#
#  AIEngine.
#
# Copyright (C) 2013  Luis Campo Giralte
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
# Written by Luis Campo Giralte <luis.camp0.2009@gmail.com> 2013
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
        self.dis.setStack(self.s)
        self.s.setTotalTCPFlows(2048)
        self.s.setTotalUDPFlows(1024)
        self.called_callback = 0 
        self.ip_called_callback = 0 

    def tearDown(self):
        del self.s
        del self.dis

    def test1(self):
        """ Create a regex for netbios and detect """
        self.s.enableLinkLayerTagging("vlan")

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("netbios","CACACACA")
        rm.addRegex(r)
        self.s.setUDPRegexManager(rm)

        self.dis.open("../pcapfiles/flow_vlan_netbios.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(r.getMatchs(), 1)

    def test2(self):
        """ Create a regex for netbios with callback """
        def callback(flow):
            self.called_callback += 1 
            r = flow.getRegex()
            self.assertEqual(r.getMatchs(),1)
            self.assertEqual(r.getName(), "netbios")
    
        self.s.enableLinkLayerTagging("vlan")

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("netbios","CACACACA")
        r.setCallback(callback)
        rm.addRegex(r)
        self.s.setUDPRegexManager(rm)

        self.dis.open("../pcapfiles/flow_vlan_netbios.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(r.getMatchs(), 1)
        self.assertEqual(self.called_callback, 1)

    def test3(self):
        """ Verify DNS and HTTP traffic """

        self.dis.open("../pcapfiles/accessgoogle.pcap");
        self.dis.run();
        self.dis.close();

        ft = self.s.getTCPFlowManager()
        fu = self.s.getUDPFlowManager()

        self.assertEqual(len(ft), 1)
        self.assertEqual(len(fu), 1)

        for flow in fu:
    	    udp_flow = flow
    	    break

        self.assertEqual(str(udp_flow.getDNSDomain()),"www.google.com")	

        for flow in ft:
    	    http_flow = flow
    	    break

        self.assertEqual(str(http_flow.getHTTPHost()),"www.google.com")

    def test4(self):
        """ Verify SSL traffic """

        self.dis.open("../pcapfiles/sslflow.pcap");
        self.dis.run();
        self.dis.close();

        ft = self.s.getTCPFlowManager()

        self.assertEqual(len(ft), 1)

        for flow in ft:
            f = flow
            break

        self.assertEqual(str(f.getSSLHost()),"0.drive.google.com")

    def test5(self):
        """ Verify SSL traffic with domain callback"""
        
        def domain_callback(flow):
            self.called_callback += 1 

        d = pyaiengine.DomainName("Google Drive Cert",".drive.google.com")
        d.setCallback(domain_callback)

        dm = pyaiengine.DomainNameManager()
        dm.addDomainName(d)

        self.s.setSSLHostNameManager(dm)

        self.dis.open("../pcapfiles/sslflow.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(dm.getTotalDomains(), 1)
        self.assertEqual(d.getMatchs() , 1)
        self.assertEqual(self.called_callback, 1)

    def test6(self):
        """ Verify SSL traffic with domain callback and IPset"""

        def ipset_callback(flow):
            self.ip_called_callback += 1

        def domain_callback(flow):
            self.called_callback += 1 

        ip = pyaiengine.IPSet("Specific IP address")
        ip.addIPAddress("74.125.24.189")
        ip.setCallback(ipset_callback)

        ipm = pyaiengine.IPSetManager()
        ipm.addIPSet(ip)

        d = pyaiengine.DomainName("Google All",".google.com")
        d.setCallback(domain_callback)

        dm = pyaiengine.DomainNameManager()
        dm.addDomainName(d)

        self.s.setTCPIPSetManager(ipm)
        self.s.setSSLHostNameManager(dm)

        self.dis.open("../pcapfiles/sslflow.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(d.getMatchs() , 1)
        self.assertEqual(self.called_callback,1)
        self.assertEqual(self.ip_called_callback,1)

    def test7(self):
        """ Attach a database to the engine """

        db = databaseTestAdaptor()

        self.s.setTCPDatabaseAdaptor(db,16)

        self.dis.open("../pcapfiles/sslflow.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(db.getInserts(), 1)
        self.assertEqual(db.getUpdates(), 5)
        self.assertEqual(db.getRemoves(), 0)

    def test8(self):
        """ Attach a database to the engine """

        db = databaseTestAdaptor()

        self.s.enableLinkLayerTagging("vlan")
        self.s.setUDPDatabaseAdaptor(db,16)

        self.dis.open("../pcapfiles/flow_vlan_netbios.pcap");
        self.dis.run();
        self.dis.close();

        self.assertEqual(db.getInserts(), 1)
        self.assertEqual(db.getUpdates(), 1)
        self.assertEqual(db.getRemoves(), 0)


    def test9(self):
        """ Attach a database to the engine and domain name"""

        def domain_callback(flow):
            self.called_callback += 1 
            self.assertEqual(str(flow.getSSLHost()),"0.drive.google.com")

        d = pyaiengine.DomainName("Google All",".google.com")

        dm = pyaiengine.DomainNameManager()
        d.setCallback(domain_callback)
        dm.addDomainName(d)

        self.s.setSSLHostNameManager(dm)

        db = databaseTestAdaptor()

        self.s.setTCPDatabaseAdaptor(db,16)

        self.dis.open("../pcapfiles/sslflow.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(db.getInserts(), 1)
        self.assertEqual(db.getUpdates(), 5)
        self.assertEqual(db.getRemoves(), 0)
        self.assertEqual(d.getMatchs(), 1)
        self.assertEqual(self.called_callback, 1)

    def test10(self):
        """ Verify iterators of the RegexManager """

        rl = [ pyaiengine.Regex("expression %d" % x, "some regex %d" % x) for x in xrange(0,5) ]

        rm = pyaiengine.RegexManager()

        [rm.addRegex(r) for r in rl] 
    
        self.s.setTCPRegexManager(rm)
        self.s.enableNIDSEngine(True)	

        self.dis.open("../pcapfiles/sslflow.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(len(rm), 5)
    
        for r in rl:
    	    self.assertEqual(r.getMatchs(), 0)

    def test11(self):
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
            ip.setCallback(ipset_callback)

            ipm = pyaiengine.IPSetManager()
            ipm.addIPSet(ip)

            self.s.setTCPIPSetManager(ipm)

            self.dis.open("../pcapfiles/sslflow.pcap");
            self.dis.run();
            self.dis.close();

            self.assertEqual(self.ip_called_callback,1)

    def test12(self):
        """ Verify the HTTP fields of the flow """

        def domain_callback(flow):
            self.called_callback += 1
            self.assertEqual(str(flow.getHTTPUri()),"/css/global.css?v=20121120a")
            self.assertEqual(str(flow.getHTTPHost()),"www.wired.com")

        d = pyaiengine.DomainName("Wired domain",".wired.com")

        dm = pyaiengine.DomainNameManager()
        d.setCallback(domain_callback)
        dm.addDomainName(d)

        self.s.setHTTPHostNameManager(dm)

        self.dis.open("../pcapfiles/two_http_flows_noending.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(self.called_callback, 1)

    def test13(self):
        """ Verify cache release functionality """

        self.s.setFlowsTimeout(50000000) # No timeout :D

        self.dis.open("../pcapfiles/sslflow.pcap")
        self.dis.run()
        self.dis.close()
        
        ft = self.s.getTCPFlowManager()

        self.assertEqual(len(ft), 1)

        for flow in ft:
            self.assertNotEqual(flow.getSSLHost(),None)
        
	self.dis.open("../pcapfiles/accessgoogle.pcap")
        self.dis.run()
        self.dis.close()

        fu = self.s.getUDPFlowManager()

        self.assertEqual(len(fu), 1)

        for flow in fu:
            self.assertNotEqual(flow.getDNSDomain(),None)

        # release some of the caches
        self.s.releaseCache("SSLProtocol")
        
        for flow in ft:
            self.assertEqual(flow.getSSLHost(),None)

        # release all the caches
        self.s.releaseCaches()

        for flow in ft:
            self.assertEqual(flow.getSSLHost(),None)
            self.assertEqual(flow.getHTTPHost(),None)

        for flow in fu:
            self.assertEqual(flow.getDNSDomain(),None)

 
class StackLanIPv6Tests(unittest.TestCase):

    def setUp(self):
        self.s = pyaiengine.StackLanIPv6()
        self.dis = pyaiengine.PacketDispatcher()
        self.dis.setStack(self.s)
        self.s.setTotalTCPFlows(2048)
        self.s.setTotalUDPFlows(1024)
        self.called_callback = 0

    def tearDown(self):
        del self.s
        del self.dis

    def test1(self):
        """ Create a regex for a generic exploit """

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("generic exploit",b"\x90\x90\x90\x90\x90\x90\x90")
        rm.addRegex(r)
        self.s.setTCPRegexManager(rm)

        self.dis.open("../pcapfiles/generic_exploit_ipv6_defcon20.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r.getMatchs(), 1)

    def test2(self):
        """ Create a regex for a generic exploit and a IPSet """
        def ipset_callback(flow):
            self.called_callback += 1 

        ipset = pyaiengine.IPSet("IPv6 generic set")
        ipset.addIPAddress("dc20:c7f:2012:11::2")
        ipset.addIPAddress("dc20:c7f:2012:11::1")
        ipset.setCallback(ipset_callback)
        im = pyaiengine.IPSetManager()

        im.addIPSet(ipset)
        self.s.setTCPIPSetManager(im)

        rm = pyaiengine.RegexManager()
        r1 = pyaiengine.Regex("generic exploit","\x90\x90\x90\x90\x90\x90\x90")
        rm.addRegex(r1)
        r2 = pyaiengine.Regex("other exploit","(this can not match)")
        rm.addRegex(r2)
        self.s.setTCPRegexManager(rm)

        self.dis.open("../pcapfiles/generic_exploit_ipv6_defcon20.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r1.getMatchs(), 1)
        self.assertEqual(r2.getMatchs(), 0)
        self.assertEqual(self.called_callback , 1)

    def test3(self):
        """ Create a regex for a generic exploit and a IPSet with no matching"""
        def ipset_callback(flow):
            self.called_callback += 1

        ipset = pyaiengine.IPSet()
        ipset.addIPAddress("dc20:c7f:2012:11::22")
        ipset.addIPAddress("dc20:c7f:2012:11::1")
        ipset.setCallback(ipset_callback)
        im = pyaiengine.IPSetManager()

        im.addIPSet(ipset)
        self.s.setTCPIPSetManager(im)

        rm = pyaiengine.RegexManager()
        r1 = pyaiengine.Regex("generic exploit","\xaa\xbb\xcc\xdd\x90\x90\x90")
        rm.addRegex(r1)
        r2 = pyaiengine.Regex("other exploit","(this can not match)")
        rm.addRegex(r2)
        self.s.setTCPRegexManager(rm)

        self.dis.open("../pcapfiles/generic_exploit_ipv6_defcon20.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r1.getMatchs(), 0)
        self.assertEqual(r2.getMatchs(), 0)
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

    def test_5(self):
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

    def test_6(self):
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

        self.s.setTCPIPSetManager(im)

        self.dis.open("../pcapfiles/generic_exploit_ipv6_defcon20.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(len(im), 3)
        self.assertEqual(self.called_callback , 0)

class StackLanLearningTests(unittest.TestCase):

    def setUp(self):
        self.s = pyaiengine.StackLan()
        self.dis = pyaiengine.PacketDispatcher()
        self.dis.setStack(self.s)
        self.s.setTotalTCPFlows(2048)
        self.s.setTotalUDPFlows(1024)
        self.f = pyaiengine.FrequencyGroup()

    def tearDown(self):
        del self.s
        del self.dis
        del self.f

    def test_1(self):

        self.f.reset()
        self.s.enableFrequencyEngine(True)

        self.dis.open("../pcapfiles/two_http_flows_noending.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(self.f.getTotalProcessFlows(), 0)
        self.assertEqual(self.f.getTotalComputedFrequencies(), 0)

        """ Add the TCP Flows of the FlowManager on the FrequencyEngine """
        ft = self.s.getTCPFlowManager()
        self.f.addFlowsByDestinationPort(ft)
        self.f.compute()
    
        self.assertEqual(self.f.getTotalProcessFlows(), 2)
        self.assertEqual(self.f.getTotalComputedFrequencies(), 1)

    def test_2(self):
        
        self.f.reset()
        self.s.enableFrequencyEngine(True)
        
        self.dis.open("../pcapfiles/tor_4flows.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(self.f.getTotalProcessFlows(), 0)
        self.assertEqual(self.f.getTotalComputedFrequencies(), 0)

        """ Add the TCP Flows of the FlowManager on the FrequencyEngine """
        ft = self.s.getTCPFlowManager()
        self.f.addFlowsByDestinationPort(ft)
        self.f.compute()

        self.assertEqual(len(self.f.getReferenceFlowsByKey("80")), 4)
        self.assertEqual(len(self.f.getReferenceFlows()), 4)
        self.assertEqual(len(self.f.getReferenceFlowsByKey("8080")), 0)
        self.assertEqual(self.f.getTotalProcessFlows(), 4)
        self.assertEqual(self.f.getTotalComputedFrequencies(), 1)

    def test_3(self):
        """ Integrate with the learner to generate a regex """
        learn = pyaiengine.LearnerEngine()

        self.f.reset()
        self.s.enableFrequencyEngine(True)
        
        self.dis.open("../pcapfiles/tor_4flows.pcap")
        self.dis.run()
        self.dis.close()

        """ Add the TCP Flows of the FlowManager on the FrequencyEngine """
        ft = self.s.getTCPFlowManager()
        self.f.addFlowsByDestinationPort(ft)
        self.f.compute()

        flow_list = self.f.getReferenceFlows()
        self.assertEqual(self.f.getTotalComputedFrequencies(), 1)
        learn.agregateFlows(flow_list)
        learn.compute()

        """ Get the generated regex and compile with the regex module """
        r = learn.getRegex()
        try:
            rc = re.compile(r)		
            self.assertTrue(True)	
        except:
            self.assertFalse(False)	

class StackVirtualTests(unittest.TestCase):

    def setUp(self):
        self.s = pyaiengine.StackVirtual()
        self.dis = pyaiengine.PacketDispatcher()
        self.dis.setStack(self.s)
        self.s.setTotalTCPFlows(2048)
        self.s.setTotalUDPFlows(1024)
        self.called_callback = 0

    def tearDown(self):
        del self.s
        del self.dis

    def test1(self):
        """ Create a regex for a detect the flow on a virtual network """

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("Bin directory","^bin$")
        rm.addRegex(r)
        self.s.setTCPRegexManager(rm)

        self.dis.open("../pcapfiles/vxlan_ftp.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r.getMatchs(), 1)

    def test2(self):
        """ Create a regex for a detect the flow on a virtual network on the GRE side """

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("Bin directory",b"^SSH-2.0.*$")
        rm.addRegex(r)
        self.s.setTCPRegexManager(rm)

        self.dis.open("../pcapfiles/gre_ssh.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r.getMatchs(), 1)
        ft = self.s.getTCPFlowManager()
        fu = self.s.getUDPFlowManager()

        self.assertEqual(len(ft), 1)
        self.assertEqual(len(fu), 0)


    def test3(self):
        """ Inject two pcapfiles with gre and vxlan traffic and verify regex """

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("SSH activity",b"^SSH-2.0.*$")
        rm.addRegex(r)
        self.s.setTCPRegexManager(rm)

        self.s.enableNIDSEngine(True)	

        # The first packet of the pcapfile is from 18 sep 2014
        self.dis.open("../pcapfiles/vxlan_ftp.pcap")
        self.dis.run()
        self.dis.close()

        """ This FlowManagers points to the virtualize layer """
        ft = self.s.getTCPFlowManager()
        fu = self.s.getUDPFlowManager()

        self.assertEqual(ft.getTotalFlows() , 1)
        self.assertEqual(ft.getTotalProcessFlows() , 1)
        self.assertEqual(ft.getTotalTimeoutFlows() , 0)

        self.assertEqual(r.getMatchs(), 0)
        self.assertEqual(len(ft), 1)
        self.assertEqual(len(fu), 0)

        self.s.setFlowsTimeout(60 * 60 * 24)

        # The first packet of the pcapfile is from 19 sep 2014
        self.dis.open("../pcapfiles/gre_ssh.pcap")
        self.dis.run()
        self.dis.close()
      
        self.assertEqual(ft.getTotalFlows() , 2)
        self.assertEqual(ft.getTotalProcessFlows() , 2)
        self.assertEqual(ft.getTotalTimeoutFlows() , 0)

        self.assertEqual(r.getMatchs(), 1)
        self.assertEqual(len(ft), 2)
        self.assertEqual(len(fu), 0)

    def test4(self):
        """ Test the extraction of the tag from the flow when matches """

        def virt_callback(flow):
            if ((flow.haveTag() == True)and(flow.getTag() == 1)):
                self.called_callback += 1

        rm = pyaiengine.RegexManager()
        r = pyaiengine.Regex("Bin directory",b"^bin$")
        r.setCallback(virt_callback)
        rm.addRegex(r)
        self.s.setTCPRegexManager(rm)

        self.s.enableNIDSEngine(True)

        self.dis.open("../pcapfiles/vxlan_ftp.pcap")
        self.dis.run()
        self.dis.close()

        self.assertEqual(r.getMatchs(), 1)
        self.assertEqual(self.called_callback,1)

if __name__ == '__main__':

    unittest.main()

    sys.exit(0)

