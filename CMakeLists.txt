cmake_minimum_required(VERSION 3.15)
project(pymae_project)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)


# maeparser
FetchContent_Declare(
  maeparser
  GIT_REPOSITORY https://github.com/schrodinger/maeparser.git
  GIT_TAG v1.3.3
)
FetchContent_MakeAvailable(maeparser)

# pybind11
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG v2.13.6
)
FetchContent_MakeAvailable(pybind11)

find_package(Boost REQUIRED)

# creating pybind module
pybind11_add_module(pymae
  pymae/pybind/module.cpp
  pymae/pybind/reader_bindings.cpp
  pymae/pybind/writer_bindings.cpp
  pymae/pybind/block_bindings.cpp
  pymae/pybind/iblock_bindings.cpp
  pymae/pybind/PythonStreambuf.cpp
)

target_include_directories(pymae
  PRIVATE
  pymae/pybind
  maeparser
  ${Boost_INCLUDE_DIRS}
)
target_link_libraries(pymae PRIVATE maeparser)