#!/bin/bash

mkdir build

conda update -n base -c defaults conda
conda install cmake ninja compilers -y

conda install -c conda-forge pdal -y
conda upgrade laz-perf -y

