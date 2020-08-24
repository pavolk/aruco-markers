
# Building from source

```
mkdir build
cd build
conan remote add conan-center "https://conan.bintray.com"
conan install -g cmake -s build_type=Debug --build=missing ..
cmake ..
cmake --build .
```