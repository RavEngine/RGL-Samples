@echo OFF
cd ..
mkdir build
mkdir build\web
cd build\web
emcmake cmake ..\.. -DCMAKE_BUILD_TYPE="Debug"
