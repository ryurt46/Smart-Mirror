cmake_minimum_required(VERSION 3.20)
project(SmartMirror LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(smart_mirror
    sw/src/main.cc
    sw/src/clock/clock.cc
    sw/src/helpers/helper.cc
    sw/src/transport/departure_group.cc
    sw/src/transport/departure.cc
    sw/src/weather/weather.cc
    sw/src/http/http_server.cc
)

target_include_directories(smart_mirror
    PRIVATE
        ${PROJECT_SOURCE_DIR}/sw/include
)

find_package(CURL REQUIRED)
target_link_libraries(smart_mirror PRIVATE CURL::libcurl)
