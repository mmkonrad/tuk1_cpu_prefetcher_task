#!/usr/bin/env bash

# Make Sure the Prefetcher is Enabled
sudo /home/Markus.Dreseler/msr.sh 0x0

# Bind Benchmark to a Single NUMA Node
numactl --cpubind=0 --membind=0 ./main > ./results/prefetch.csv

# Disable Prefetcher
sudo /home/Markus.Dreseler/msr.sh 0xf
numactl --cpubind=0 --membind=0 ./main > ./results/non-prefetch.csv

# Reactivate Prefetcher after Benchmark
sudo /home/Markus.Dreseler/msr.sh 0x0

# Plot Results
python3 ./plot/plot.py