#!/bin/sh
mkdir ../Build
cd ../Build
cmake .. -DFOG_BUILD_UISYSTEM_X11=1 -DCMAKE_BUILD_TYPE=Debug -DFOG_BUILD_BENCH=True -G"Unix Makefiles"
cd ../Scripts
