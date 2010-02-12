mkdir ..\Build
cd ..\Build
cmake .. -DCMAKE_BUILD_TYPE=Release -DFOG_BUILD_BENCH=True -G"Visual Studio 6"
cd ..\Scripts
pause
