cmake_minimum_required(VERSION 3.20)

project(compiler-on-cpp LANGUAGES CXX)

add_executable(compiler main.cpp)

target_compile_features(compiler PRIVATE cxx_std_20)
