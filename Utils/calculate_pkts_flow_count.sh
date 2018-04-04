#!/bin/bash

cut -d"," -f4,5 ../files/pkts.csv | tr "," ":" | cut -d":" -f2,4 | sort | uniq -c | sort -n -r > ../files/pkts_flows_count.csv

