#!/bin/bash

if [[ $# -eq 2 ]]
then
    FILE="$1"
    RES_FILE="$2"
else
    FILE="../datasets_files/equinix-chicago.dirA.20160121-130000.UTC.anon.pcap"
    RES_FILE="../datasets_files/CAIDA/caida.csv"
fi

tcpdump -v -r ${FILE} -n | awk 'BEGIN {accum_line = "";} /^..:/{if(length(accum_line)){print accum_line; accum_line = "";}} {accum_line = accum_line " " $0;} END {if(length(accum_line)){print accum_line; }}' | grep " IP " | sed -E "s/.*id ([^,]*),.* proto ([^,]*),.* length ([^,]*)\)     ([^.]*\.[^.]*\.[^.]*\.[^.]*)\.*(.*) > ([^.]*\.[^.]*\.[^.]*\.[^.:]*)\.*([^:]*).*/\1,\2,\3,\4,\6,\5,\7/" >> ${RES_FILE}
