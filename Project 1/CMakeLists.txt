cmake_minimum_required(VERSION 3.23)
project(cz4031-project-1)

set(CMAKE_CXX_STANDARD 20)

add_executable(main src/main.cpp src/disk.cpp src/disk.h src/tree.cpp src/tree.h src/dtypes.h src/tree_remove.cpp src/tree_search.cpp src/tree_insert.cpp src/tree_display.cpp)
