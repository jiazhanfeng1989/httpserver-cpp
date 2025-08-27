cmake_minimum_required(VERSION 3.11)
set(HTTP_SERVER_DEMO_TARGET httpserver_demo)

set(DEMO_FILES ${PROJECT_SOURCE_DIR}/demo/main.cpp)

add_executable(${HTTP_SERVER_DEMO_TARGET} ${DEMO_FILES})

target_link_libraries(${HTTP_SERVER_DEMO_TARGET}
    ${HTTP_SERVER_TARGET}
    boost_url
)
