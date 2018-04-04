#!/bin/bash

cut -d"," -f2,3 ../files/UCLA/lasr.cs.ucla.edu/ddos/traces/public/trace5/UCLA5.csv | tr "," ":" | sort | uniq -c | sort -n -r > ../files/UCLA5_flows_count.csv
awk '{s+=$1}END{print s}' ../files/UCLA5_flows_count.csv > ./UCLA5_total_flows
cat ./UCLA5_total_flows ../files/UCLA5_flows_count.csv > ./tmp
rm ./UCLA5_total_flows
mv ./tmp ../files/UCLA5_flows_count.csv
