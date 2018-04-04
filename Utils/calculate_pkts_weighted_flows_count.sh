#!/bin/bash
cut -d"," -f3,4,5 ../files/pkts.csv | sed -E 's/length:(.*),IP_SRC:(.*),IP_DST:(.*)/\2:\3 \1/' | awk '{a[$1] += $2} END {for (i in a) { printf "%s %s\n", i ,a[i] | "sort -rnk2";}}' > ../files/pkts_weighted_flows_count.csv

