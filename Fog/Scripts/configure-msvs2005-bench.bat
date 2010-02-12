mkdir ..\Build
cd ..\Build
cmake .. -DCMAKE_BUILD_TYPE=Release -DFOG_BUILD_BENCH=True -G"Visual Studio 8 2005"
cd ..\Scripts
pause
