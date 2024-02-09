#!/bin/bash

export PKG_CONFIG_PATH=$CONDA_PREFIX/lib64/pkgconfig

LDFLAGS="$LDFLAGS -Wl,-rpath-link,$CONDA_PREFIX/lib" cmake -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=${CONDA_PREFIX} \
      ..
