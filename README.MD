# Build Instructions

```
// build shared library

mkdir build
cd build 
cmake ..
make

// Build Demo Program
g++ -Wall -g -c ../src/main.cpp -I../include `pkg-config --cflags --libs gstreamer-1.0`

// Link library to demo program to make an executable
g++ -Wall -Wl,-rpath,. -o main main.o libGSTPipelineApp.so  `pkg-config --libs gstreamer-1.0 gstreamer-app-1.0`

// run executable
./main

``` 