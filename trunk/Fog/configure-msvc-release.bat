mkdir build_rel
cd build_rel
cmake .. -DCMAKE_BUILD_TYPE=Release -DFOG_DEBUG=0 -G"Visual Studio 8 2005"
cd ..
pause
