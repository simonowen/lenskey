cmake_minimum_required(VERSION 3.12...3.30)

project(LensKey LANGUAGES CXX)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(FATAL_ERROR "Sorry, this project is currently Windows only.")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} WIN32 LensKey.cpp LensKey.rc)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive>
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>: -Wpedantic -Wall -Wextra -Wconversion>
)
