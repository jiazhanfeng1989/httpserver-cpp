# spdlog usage

[spdlog](https://github.com/gabime/spdlog) is a popular C++ header only logging library.

## spdlog installation:
```bash
1. git clone --depth 1 -b v1.15.3 https://github.com/gabime/spdlog.git spdlog_1_15_3
2. cd spdlog_1_15_3
3. mkdir build && cd build
4. cmake -DCMAKE_INSTALL_PREFIX=${HOME}/spdlog_1_15_3_custom ..
5. make -j4
6. make install
```
## spdlog in CMakeLists.txt:
```cmake
find_package(spdlog 1.15.3 QUIET)
if(NOT spdlog_FOUND)
    # If not found, set custom paths and try again
    message(STATUS "spdlog cmake not found in default path, try to use custom spdlog cmake path")
    message(STATUS "spdlog custom cmake path: $ENV{HOME}/spdlog_1_15_3_custom/share/cmake/spdlog")
    set(spdlog_DIR "$ENV{HOME}/spdlog_1_15_3_custom/share/cmake/spdlog")
    find_package(spdlog 1.15.3 REQUIRED)
else()
    message(STATUS "spdlog found in default path")
    find_package(spdlog 1.15.3 REQUIRED)
endif()

```