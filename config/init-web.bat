@echo OFF
cd ..
mkdir build
mkdir build\web
cd build\web
..\..\..\emsdk\upstream\emscripten\emcmake.bat cmake -Wno-dev -DCMAKE_TOOLCHAIN_FILE=../../../emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ../..
