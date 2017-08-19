#! /bin/bash

mkdir build
cd build
cmake ..
make
cd ../bin
echo "======================="
./mydemo