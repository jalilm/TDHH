#!/bin/bash

tcpdump ip -v -r ../datasets_files/equinix-chicago.dirA.20160121-130000.UTC.anon.pcap -n | awk 'BEGIN {accum_line = "";} /^..:/{if(length(accum_line)){print accum_line; accum_line = "";}} {accum_line = accum_line " " $0;} END {if(length(accum_line)){print accum_line; }}' | grep -v IP6 | grep IP | sed -E "s/.*id ([^,]*),.* proto ([^,]*),.* length ([^,]*)\)     ([^.]*\.[^.]*\.[^.]*\.[^.]*)\.*(.*) > ([^.]*\.[^.]*\.[^.]*\.[^.:]*)\.*([^:]*).*/id:\1,proto:\2,length:\3,IP_SRC:\4,IP_DST:\6,PORT_SRC:\5,PORT_DST:\7/" > ../datasets_files/CAIDA16/caida.csv

