mkdir ../build
cd ../build
mkdir ios
cd ios
cmake -G "Xcode" -DCMAKE_TOOLCHAIN_FILE="../../config/ios.toolchain.cmake" -DPLATFORM="OS64COMBINED" -DDEPLOYMENT_TARGET=14.0 ../..
