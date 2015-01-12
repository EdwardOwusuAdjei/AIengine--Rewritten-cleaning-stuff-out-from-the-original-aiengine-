#!/usr/bin/env python

""" Example for extract the most used DNS request of a network """

__author__ = "Luis Campo Giralte"
__copyright__ = "Copyright (C) 2013-2015 by Luis Campo Giralte"
__revision__ = "$Id$"
__version__ = "0.1"
import sys
import os
sys.path.append("../src/")
import pyaiengine

top_ips = dict()

def callback_host(flow):

    ip = str(flow).split(":")[0]

    if(top_ips.has_key(ip)):
        top_ips[ip] += 1
    else:
        top_ips[ip] = 1

if __name__ == '__main__':

    # Load an instance of a Network Stack on Lan network
    st = pyaiengine.StackLan()

    # Create a instace of a PacketDispatcher
    pdis = pyaiengine.PacketDispatcher()

    # Plug the stack on the PacketDispatcher
    pdis.setStack(st)

    dm = pyaiengine.DomainNameManager()

    dom = pyaiengine.DomainName("Service to analyze",
        "marca.com")
    dom.setCallback(callback_host)
    dm.addDomainName(dom)

    st.setHTTPHostNameManager(dm)

    st.setTotalTCPFlows(327680)
    st.setTotalUDPFlows(163840)

    pdis.open("eth0")

    try:
        pdis.run()
    except:
        e = sys.exc_info()[0]
        print("Interrupt during capturing packets:",e)

    pdis.close()

    # Dump on file the statistics of the stack
    st.setStatisticsLevel(5)
    f = open("statistics.log","w")
    f.write(str(st))
    f.close()
    
    print(top_ips)

    sys.exit(0)

