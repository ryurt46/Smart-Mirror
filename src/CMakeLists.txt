cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Lägg till alla källfiler
set(SOURCES
    main.cc
    clock/clock.cc
    weather/weather.cc
    helpers/helper.cc
    transport/transport.cc
)

add_executable(my_app ${SOURCES})
find_package(CURL REQUIRED)
target_link_libraries(my_app PRIVATE CURL::libcurl)
# Om du använder nlohmann/json som header-only
# lägg till include-sökvägen
# include_directories(weather/external/json/include)  # ändra om du har json.hpp där
