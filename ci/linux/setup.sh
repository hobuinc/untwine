#!/bin/bash

mkdir build

conda update -n base -c defaults conda
#conda install -c conda-forge cmake ninja compilers -y
conda install -c conda-forge cmake ninja -y
conda install -c conda-forge --yes --quiet pdal -y

