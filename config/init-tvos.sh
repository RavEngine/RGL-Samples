mkdir ../build
cd ../build
mkdir tvos
cd tvos
cmake -G "Xcode" -DCMAKE_SYSTEM_NAME="tvOS" ../..
