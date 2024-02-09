#!/bin/bash

cmake -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_FIND_FRAMEWORK="NEVER" \
      -DCMAKE_INSTALL_PREFIX=${CONDA_PREFIX} \
      ..
