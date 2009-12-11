mkdir Build
cd Build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DFOG_DEBUG=1 -DFOG_DEBUG_MEMORY=1 -G"MinGW Makefiles"
cd ..
pause
