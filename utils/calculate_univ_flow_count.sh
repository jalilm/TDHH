#!/bin/bash

TRACE_FILE="univ1.csv"
DIR="../datasets_files/UNIV1/"
RES_PREFIX="univ_flows_count"
TOTAL_FILE="univ_total_flows"
STEP=1000000

count=0
lines=`cat ${DIR}${TRACE_FILE} | wc -l`
sizes=`seq 0 ${STEP} ${lines}`
touch ${DIR}${RES_PREFIX}-0.csv;

for i in ${sizes}; do
    let si=i+1;
    let j=i+${STEP};
    let k=j+1;
    sed -n "${si},${j}p;${k}q" ${DIR}${TRACE_FILE} | cut -d"," -f4,5 | tr "," ":" | sort | uniq -c | sort -n -r > ${DIR}${RES_PREFIX}-${j}.csv;
    let count+=`awk '{s+=$1}END{print s}' ${DIR}${RES_PREFIX}-${j}.csv`;
    echo ${count} > ${DIR}${TOTAL_FILE};
    cat ${DIR}${TOTAL_FILE} ${DIR}${RES_PREFIX}-${j}.csv > ${DIR}tmp;
    tail -q -n +2 ${DIR}${RES_PREFIX}-${i}.csv ${DIR}tmp | awk '{a[$2] += $1} END {for (i in a) { printf "%s %s\n", a[i] ,i | "sort -rn";}}' > ${DIR}${RES_PREFIX}-${j}.csv
    cat ${DIR}${TOTAL_FILE} ${DIR}${RES_PREFIX}-${j}.csv > ${DIR}tmp;
    mv ${DIR}tmp ${DIR}${RES_PREFIX}-${j}.csv;
    rm ${DIR}${TOTAL_FILE};
done
