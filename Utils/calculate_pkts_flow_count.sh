#!/bin/bash

cut -d"," -f4,5 ../files/pkts.csv | tr "," ":" | cut -d":" -f2,4 | sort | uniq -c | sort -n -r > ../files/pkts_flows_count.csv
awk '{s+=$1}END{print s}' ../files/pkts_flows_count.csv > ./pkts_total_flows
cat ./pkts_total_flows ../files/pkts_flows_count.csv > ./tmp
rm ./pkts_total_flows
mv ./tmp ../files/pkts_flows_count.csv
