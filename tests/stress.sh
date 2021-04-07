#!/bin/sh
#
#Date: Thu, 14 Apr 2011 12:59:14 -0500
# From: Chris_Poblete@Dell.com
#
# Then run the test script in multiple processes:
# for ((ii=0; ii<6; ii++)); do ./stress.sh & done
while true; do
  wsman enumerate http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem -h localhost -P 5985 -u wsman -p secret -j utf-8 -y basic -v -o -m 256 2>&1 >>/tmp/$$.log
#  sleep 1
done