    cmake_minimum_required(VERSION 3.10)
    project(ASTViz)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    include_directories(include)

    find_package(raylib REQUIRED)

    add_executable(plot plot.cpp)

    target_link_libraries(plot PRIVATE raylib)
