cmake_minimum_required(VERSION 2.8.9)
project(SQL_interpretator)

# C++17 fixes problem for constexpr static member
# otherwise I catch the error:
# task10_4/server.cpp:30: undefined reference to `Server::stop_message'
set(CMAKE_CXX_STANDARD 17)

# библиотека для работы тредов и семафоров
SET(CMAKE_CXX_FLAGS "-pthread")

# можно писать инклюды в формате:
# include <server/server.h> а не include "......./server.h"
# т.е. препроцессор будет искать фаилы в директории src
include_directories(${PROJECT_SOURCE_DIR}/src)

# путь к директории с ресурсами .txt и проч.
# example: define HELP_FILE  _cmake_assistive_dir"/help_info.txt"
add_definitions(-D_cmake_assistive_dir="${PROJECT_SOURCE_DIR}/assistive")

# билдим основные исходники проекта
# можно собрать симейком в src
add_subdirectory(src)
# или можно собрать кучей маленьких симейков
#add_subdirectory(src/analyze)
#add_subdirectory(src/client)
#add_subdirectory(src/exeption)
#add_subdirectory(src/server)
#add_subdirectory(src/table)


# создаем исполняемый фаил server_run
add_executable(server_run run_server.cpp)
target_link_libraries(server_run Server Exeption Analyze)


# создаем исполняемый фаил client_run
add_executable(client_run run_client.cpp)
target_link_libraries(client_run Client Exeption)

## Build tests
set(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject")
set(BUILD_GMOCK ON CACHE BOOL "Builds the googlemock subproject")
set(gtest_disable_pthreads ON CACHE BOOL "Disable uses of pthreads in gtest.")
add_subdirectory(external_library/googletest-release-1.8.0)
enable_testing()
add_subdirectory(tests)