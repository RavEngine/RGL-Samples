@echo OFF

cd ..
mkdir build
cd build
mkdir winarm
cd winarm
cmake -AARM64 ..\..
cd ..\..
cd config