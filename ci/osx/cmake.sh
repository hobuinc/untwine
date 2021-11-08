#!/bin/bash


git clone https://github.com/hobu/laz-perf.git laz-perf && cd laz-perf \
    mkdir build && cd build && \
    cmake -G Ninja .. -DWITH_TESTS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo  -Dgtest_force_shared_crt=ON -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX && \
    ninja install && \
    cd ..

cmake .. \
      -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_LIBRARY_PATH:FILEPATH="$CONDA_PREFIX/lib" \
      -DCMAKE_INCLUDE_PATH:FILEPATH="$CONDA_PREFIX/include" \
      -DCMAKE_FIND_FRAMEWORK="NEVER" \
      -DCMAKE_INSTALL_PREFIX=${CONDA_PREFIX}
