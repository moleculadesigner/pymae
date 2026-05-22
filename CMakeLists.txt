cmake_minimum_required(VERSION 3.15)
project(pymae_project)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF) # Чтобы видеть прогресс скачивания в терминале


# maeparser
FetchContent_Declare(
  maeparser
  GIT_REPOSITORY https://github.com/schrodinger/maeparser.git
  GIT_TAG v1.3.3
)
FetchContent_MakeAvailable(maeparser)
FetchContent_GetProperties(maeparser)
#get_target_property(MAE_INCLUDE_DIR maeparser SOURCE_DIR)
#if(NOT MAE_INCLUDE_DIR)
#    set(MAE_INCLUDE_DIR "${CMAKE_BINARY_DIR}/_deps/maeparser-src")
#endif()

# pybind11
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG v2.13.6
)
FetchContent_MakeAvailable(pybind11)

# creating pybind module
pybind11_add_module(pymae pymae/main.cpp)

# Линкуем maeparser (цель называется maeparser)
target_include_directories(pymae PRIVATE (${maeparser_SOURCE_DIR}))
target_link_libraries(pymae PRIVATE maeparser)