#!/bin/sh
mkdir ../Build
cd ../Build
cmake .. -DFOG_BUILD_UISYSTEM_X11=1 -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles"
cd ../Scripts
