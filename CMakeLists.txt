cmake_minimum_required(VERSION 3.23)
project(myfs)

set(CMAKE_CXX_STANDARD 20)

add_executable(
        myfs
        main.cpp
        commands.cpp
        commands.h
        pseudofat.cpp
        pseudofat.h
)
