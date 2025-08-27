cmake_minimum_required(VERSION 3.11)

find_package(doctest 2.4 QUIET) 
if(NOT doctest_FOUND)
    # If not found, set custom paths and try again
    message(STATUS "doctest cmake not found in default path, try to use custom doctest cmake path")
    message(STATUS "doctest custom cmake path: $ENV{HOME}/doctest_2_4_12_custom/share/cmake/doctest")
    set(doctest_DIR "$ENV{HOME}/doctest_2_4_12_custom/lib/cmake/doctest")
    find_package(doctest 2.4 REQUIRED)
else()
    message(STATUS "doctest found in default path")
    find_package(doctest 2.4 REQUIRED)
endif()

func_collect_source_files(SRC_FILES ${PROJECT_SOURCE_DIR}/test)

set(HTTP_SERVER_TEST_TARGET http_server_test)
add_executable(${HTTP_SERVER_TEST_TARGET} ${SRC_FILES})

target_include_directories(${HTTP_SERVER_TEST_TARGET}
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

func_link_libraries(${HTTP_SERVER_TEST_TARGET}
    ${HTTP_SERVER_TARGET}
    doctest::doctest
)

add_test(NAME ${HTTP_SERVER_TEST_TARGET} COMMAND ${HTTP_SERVER_TEST_TARGET})