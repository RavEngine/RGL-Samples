mkdir ../build
cd ../build
mkdir ios
cd ios
cmake -G "Xcode" -DCMAKE_SYSTEM_NAME="iOS" ../..
