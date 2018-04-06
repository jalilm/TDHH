#!/bin/bash

cut -d"," -f3,4,5 ../files/pkts.csv | sed -E 's/length:(.*),IP_SRC:(.*),IP_DST:(.*)/\1 \2:\3/' | awk '{a[$2] += $1} END {for (i in a) { printf "%s %s\n", a[i] ,i | "sort -rn";}}' > ../files/pkts_weighted_flows_count.csv
awk '{s+=$1}END{print s}' ../files/pkts_weighted_flows_count.csv > ./pkts_total_weight
cat ./pkts_total_weight ../files/pkts_weighted_flows_count.csv > ./tmp
rm ./pkts_total_weight
mv ./tmp ../files/pkts_weighted_flows_count.csv

