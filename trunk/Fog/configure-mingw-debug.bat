mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=debug -DDEBUG=1 -DDEBUG_MEMORY=1 -DDEBUG_VALGRIND=1 -G"MinGW Makefiles"
cd ..
pause
