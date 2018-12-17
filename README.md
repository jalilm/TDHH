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

# How to run the program
Simply run any variation fo the following command:
./TDHH {VE, FE, HH} {CAIDA, CAIDA18, UCLA, UCLA_FULL, UNIV1, UNIV2}

The first paramter sets the application to run:
* VE = Volume Estimation
* FE = Frequency Estimation
* HH = Heavy Hitters

The second parameter sets which trace to use:
* CAIDA: for CAIDA'16 traces.
* CAIDA18: for CAIDA'18 traces.
* UCLA_FULL: for all of the UCLA traces.
* UNIV1: for the UNIV1 trace.
* UNIV2: for the UNIV2 trace.
