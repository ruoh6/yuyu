#!/bin/bash

if [ -d "build" ]; then \
    cd build && make -j4; \
else \
    mkdir build; \
    cd build && cmake ../ -DCMAKE_C_COMPILE=/usr/lib/ccache/gcc -DCMAKE_CXX_COMPILE=/usr/lib/ccache/g++ ;\
fi
