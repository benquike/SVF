This is a new version of SVF with an improved internal design to support
such features as unittests and etc.

# building SVF

On Ubuntu 20.04, use the following commands:

``` sh
cd <somewhere>
git clone https://github.com/benquike/SVF
git submodule init; git submodule update
cd SVF
mkdir build
cd build_debug
CC=clang-10 CXX=clang++-10 LLVM_DIR=/usr/lib/llvm-10 cmake \
-DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=On ..
```
To build with release type, pass `-DCMAKE_BUILD_TYPE=Release`.

To build with ASAN, use the following CMake command:

``` sh
CC=clang-10 CXX=clang++-10 LLVM_DIR=/usr/lib/llvm-10 \
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=On \
-DSVF_BUILD_UNITTESTS=On \
-DCMAKE_CXX_FLAGS="-fno-omit-frame-pointer -fsanitize=address" \
-DCMAKE_C_FLAGS="-fno-omit-frame-pointer -fsanitize=address" \
-DCMAKE_MODULE_LINKER_FLAGS="-fsanitize=address" ..
```


## Other CMake build parameters

| Parameter           | Type        | Description     |
| -----------         | ----------- | ----------      |
| SVF_BUILD_UNITTESTS | BOOL        | Build unittests |


# building doxygen document

If you want to build the documentation yourself go into doc and invoke doxygen:

```sh
cd doc && doxygen doxygen.config
```
