#!/bin/bash

mkdir build

conda update -n base -c defaults conda
conda install cmake ninja compilers -y

conda install -c conda-forge laz-perf=2.1.0 pdal -y

