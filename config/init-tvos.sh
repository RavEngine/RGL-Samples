mkdir ../build
cd ../build
mkdir tvos
cd tvos
cmake -G "Xcode" -DCMAKE_TOOLCHAIN_FILE="../../config/ios.toolchain.cmake" -DPLATFORM="TVOSCOMBINED" -DDEPLOYMENT_TARGET=14.0 ../..
