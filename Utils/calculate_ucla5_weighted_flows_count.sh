#!/bin/bash

cat ../files/UCLA/lasr.cs.ucla.edu/ddos/traces/public/trace5/weighted_UCLA5.csv | tr 'U' ',' | tr -s ',' | cut -d"," -f2,3,6 | sed -E 's/(.*),(.*),(.*)/\3 \1:\2/' | awk '{a[$2] += $1; a[$2] += 64} END {for (i in a) { printf "%s %s\n", a[i] ,i | "sort -rn";}}' > ../files/UCLA5_weighted_flows_count.csv
awk '{s+=$1}END{print s}' ../files/UCLA5_weighted_flows_count.csv > ./ucla5_total_weight
cat ./ucla5_total_weight ../files/UCLA5_weighted_flows_count.csv > ./tmp
rm ./ucla5_total_weight
mv ./tmp ../files/UCLA5_weighted_flows_count.csv

