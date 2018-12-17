# Network-Wide Routing-Oblivious Heavy Hitters source code (TDHH)
This repository contains the source code of the "Network-Wide Routing-Oblivious Heavy Hitters", by Ran Ben Basat, Gil Einziger, Shir Landau Feibish, Jalil Moraney, and Danny Raz (ACM/IEEE ANCS 2018).

Implementation was done by Jalil Moraney (jalilm[at]cs.technion.ac.il), 2018.

This code performs volume estimation, frequency estimation, and heavy hitter detection, as described in the paper. The code works on several real-life traces and can be easily extended to other traces.

# Minimal Requirement
Any system that have the following:
* cmake version >= 3.5.1
* boost library version >= 1.58.0
* C++11 compiler

# How to compile
Simply run in the root directory:
* cmake .
* make

These commands produce the exectuable TDHH in the root directory.

# Obtaining the traces
Before being able to reproduce the results, you will have to obtain the datasets traces from several locations and pre-process them.

* You will have to request access to CAIDA aannonymized passive traces 2016 from CAIDA website:

https://www.caida.org/data/passive/passive_dataset_request.xml

and the download the Jan, 21st traces using and your credintails:

https://data.caida.org/datasets/passive-2016

* Download the UCLA traces (1 to 10) using the following links:

https://lasr.cs.ucla.edu/ddos/traces/public/trace1/

https://lasr.cs.ucla.edu/ddos/traces/public/trace2/

https://lasr.cs.ucla.edu/ddos/traces/public/trace3/

https://lasr.cs.ucla.edu/ddos/traces/public/trace4/

https://lasr.cs.ucla.edu/ddos/traces/public/trace5/

https://lasr.cs.ucla.edu/ddos/traces/public/trace6/

https://lasr.cs.ucla.edu/ddos/traces/public/trace7/

https://lasr.cs.ucla.edu/ddos/traces/public/trace8/

https://lasr.cs.ucla.edu/ddos/traces/public/trace9/

https://lasr.cs.ucla.edu/ddos/traces/public/trace10/

* Download the UNIV1 traces using:

http://pages.cs.wisc.edu/~tbenson/IMC10_Data.html 

You will have to uncompress the compressed file and put them in the appropriate directory under ./datasets_files before pre processing them in the next step.

# Pre processing the traces:
After obtaining the traces, navigate into ./utils using "cd ./utils" and perform the following:

* For CAIDA traces:
1) For each pcap file of intrest in the CAIDA trace run the follwoing:
  
  ./extract_script.sh ../datasets_files/CAIDA/equinix-chicago.dirA.20160121-130000.UTC.anon.pcap ../datasets_files/CAIDA/caida.csv
  
2) Calculate flow counts using:
  
  ./calculate_caida_flow_count.sh

* For UCLA traces:
1) For each x.pcap file, run the following:
  
  ./extract_script.sh ../datasets_diles/UCLA_FULL/x.pcap ../datasets_files/UCLA_FULL/ucla_full.csv
  
2) Calculate flow counts using:
  
  ./calculate_ucla_flow_count.sh

* For UNIV1 traces:
1) For each x.pcap file, run the following:
  
  ./extract_script.sh ../datasets_diles/UNIV1/x.pcap ../datasets_files/UNIV1/univ1.csv
  
2) Calculate flow counts using:
  
  ./calculate_univ_flow_count.sh

# How to run the program
Simply run any variation fo the following command:

./TDHH {VE, FE, HH} {CAIDA, CAIDA18, UCLA, UCLA_FULL, UNIV1, UNIV2}

The first paramter sets the application to run:
* VE = Volume Estimation
* FE = Frequency Estimation
* HH = Heavy Hitters

The second parameter sets which trace to use:
* CAIDA: for CAIDA'16 traces.
* UCLA_FULL: for the UCLA traces.
* UNIV1: for the UNIV trace.

# Obtaining the results
Each run writes to a specified output file under "./results", the file name is the following:

./{ve,fe,hh}\_{caida,ucla,univ1}.raw\_res
