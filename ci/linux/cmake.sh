#!/bin/bash

export PKG_CONFIG_PATH=$CONDA_PREFIX/lib64/pkgconfig

git clone https://github.com/hobu/laz-perf.git laz-perf && cd laz-perf && \
    mkdir build && cd build && \
    cmake -G Ninja .. -DWITH_TESTS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo  -Dgtest_force_shared_crt=ON -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX && \
    ninja install && \
    cd ../..

pwd


LDFLAGS="$LDFLAGS -Wl,-rpath-link,$CONDA_PREFIX/lib" cmake -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=${CONDA_PREFIX} \
      ..
