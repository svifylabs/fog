mkdir build_rel
cd build_rel
cmake .. -DCMAKE_BUILD_TYPE=Release -DMAKE_BENCH=True -G"Visual Studio 8 2005"
cd ..
pause
