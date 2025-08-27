# zlib usage

[zlib](https://github.com/madler/zlib) is a popular C library for data compression.

## zlib installation:
```bash
1. git clone --depth 1 -b v1.3.1 https://github.com/madler/zlib.git zlib_1_3_1
2. cd zlib_1_3_1
3. mkdir build && cd build
4. cmake -DCMAKE_INSTALL_PREFIX=${HOME}/zlib_1_3_1_custom ..
5. make -j4
6. make install
```
## zlib in CMakeLists.txt:
```cmake
find_package(PkgConfig REQUIRED)
find_package(ZLIB 1.3.1 QUIET)
if(NOT ZLIB_FOUND)
    message(STATUS "ZLIB not found in default path, try to use custom ZLIB path")
    message(STATUS "ZLIB custom path: $ENV{HOME}/zlib_1_3_1_custom")
    set(ZLIB_ROOT "$ENV{HOME}/zlib_1_3_1_custom")
       
    # Set PKG_CONFIG_PATH for pkgconfig to find zlib.pc
    set(ENV{PKG_CONFIG_PATH} "$ENV{HOME}/zlib_1_3_1_custom/share/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    
    # Try with pkg-config first
    pkg_check_modules(PC_ZLIB QUIET zlib)
    if(PC_ZLIB_FOUND)
        set(ZLIB_INCLUDE_DIRS ${PC_ZLIB_INCLUDE_DIRS})
        set(ZLIB_LIBRARIES ${PC_ZLIB_LIBRARIES})
        set(ZLIB_LIBRARY_DIRS ${PC_ZLIB_LIBRARY_DIRS})
        set(ZLIB_FOUND TRUE)
        message(STATUS "Found ZLIB via pkg-config: ${ZLIB_LIBRARIES}")
    else()
        # Fallback to manual discovery
        find_path(ZLIB_INCLUDE_DIR zlib.h HINTS $ENV{HOME}/zlib_1_3_1_custom/include)
        find_library(ZLIB_LIBRARY NAMES z zlib HINTS $ENV{HOME}/zlib_1_3_1_custom/lib)
        
        if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)
            set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})
            set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
            set(ZLIB_FOUND TRUE)
            message(STATUS "Found ZLIB manually: ${ZLIB_LIBRARY}")
        else()
            message(FATAL_ERROR "Could not find ZLIB in custom path")
        endif()
    endif()
    
    # Create imported target if it doesn't exist
    if(ZLIB_FOUND AND NOT TARGET ZLIB::ZLIB)
        add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
        set_target_properties(ZLIB::ZLIB PROPERTIES
            IMPORTED_LOCATION "${ZLIB_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}")
    endif()
else()
    message(STATUS "ZLIB found in default path")
endif()
```