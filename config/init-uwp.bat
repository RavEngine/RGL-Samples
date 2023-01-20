@echo OFF

cd ..
mkdir build
cd build
mkdir uwp
cd uwp
cmake -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION="10.0" ..\..
cd ..\..
cd config