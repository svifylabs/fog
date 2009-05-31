mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DDEBUG=1 -DDEBUG_MEMORY=1 -DDEBUG_VALGRIND=1 -G"Visual Studio 8 2005"
cd ..
pause
