## Project Setting
cmake_minimum_required(VERSION 3.15...3.29)
project(sabre LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Build type not set - defaulting to Release")
  set(CMAKE_BUILD_TYPE Release)
endif()

# Set CMake module path to include custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

if (MSVC)
  cmake_policy(SET CMP0167 NEW)
  add_compile_options(/wd4244) # Disable warning C4244 (Conversion from 'int' to 'float', possible loss of data)
  add_compile_options(/wd4819) # Disable warning C4819 (Code page in Unicode build)
endif()

# Find Boost
find_package(Boost REQUIRED)
if(Boost_FOUND)
  if(NOT TARGET Boost::headers)
    add_library(Boost::headers IMPORTED INTERFACE)
    if(TARGET Boost::boost)
      # Classic FindBoost
      set_property(TARGET Boost::boost PROPERTY INTERFACE_LINK_LIBRARIES Boost::boost)
    else()
      # Very old FindBoost, or newer Boost than CMake in older CMakes
      set_property(TARGET Boost::headers PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
    endif()
  endif()
endif()


# FetchContent for dependencies
include(FetchContent)

# Eigen3
set(EIGEN_BUILD_DOC OFF CACHE BOOL "Disable Eigen documentation")
set(EIGEN_BUILD_UNSUPPORTED OFF CACHE BOOL "Disable Eigen unsupported module")
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "Disable Eigen pkgconfig")
set(EIGEN_BUILD_BENCHMARKS OFF CACHE BOOL "Disable Eigen benchmarks")
set(BUILD_TESTING OFF CACHE BOOL "Disable all testing") # Autodiff will conflict with Eigen's testing
FetchContent_Declare(
    Eigen3
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG 3.4.0
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(Eigen3)


# autodiff
set(AUTODIFF_BUILD_TESTS OFF CACHE BOOL "Disable autodiff tests")
set(AUTODIFF_BUILD_PYTHON OFF CACHE BOOL "Disable autodiff Python bindings")
set(AUTODIFF_BUILD_EXAMPLES OFF CACHE BOOL "Disable autodiff examples")
FetchContent_Declare(
    autodiff
    GIT_REPOSITORY https://github.com/autodiff/autodiff
    GIT_TAG v1.0.3
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(autodiff)

# Find Graphviz
# option(BUILD_WITH_GRAPHVIZ "Build with Graphviz support" OFF)
set(BUILD_WITH_GRAPHVIZ OFF CACHE BOOL "Disable Graphviz support")
message(STATUS "BUILD_WITH_GRAPHVIZ is set to ${BUILD_WITH_GRAPHVIZ}")

# set(GRAPHVIZ_ROOT "" CACHE PATH "Path to Graphviz installation")
if (BUILD_WITH_GRAPHVIZ)
  find_package(Graphviz REQUIRED)
  if(GRAPHVIZ_FOUND)
    include_directories(${GRAPHVIZ_INCLUDE_DIRS})
    link_directories(${GRAPHVIZ_LIBRARY_DIRS})
    add_definitions(-DWITH_GRAPHVIZ)
    message(STATUS "Graphviz library directories: ${GRAPHVIZ_LIBRARY_DIRS}")
    message(STATUS "Graphviz include directories: ${GRAPHVIZ_INCLUDE_DIRS}")
  else()
    message(FATAL_ERROR "Graphviz not found")
  endif()
else()
  message(STATUS "Graphviz support disabled")
endif()


# File filter
set(QSTEED_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/qsteedcpp")
set(QARSER_DIR "${QSTEED_CPP_DIR}/Qarser")
set(SABRE_DIR "${QSTEED_CPP_DIR}/SABRE")
set(DAG_DIR "${QSTEED_CPP_DIR}/DAG")
set(GATES_DIR "${QSTEED_CPP_DIR}/Gates")

file(GLOB_RECURSE QARSER_SOURCES "${QARSER_DIR}/*.cpp")
file(GLOB_RECURSE SABRE_SOURCES "${SABRE_DIR}/*.cpp")
file(GLOB_RECURSE DAG_SOURCES "${DAG_DIR}/*.cpp")
file(GLOB_RECURSE GATES_SOURCES "${GATES_DIR}/*.cpp")

include_directories(
  "${QSTEED_CPP_DIR}"
)

# 0. Build Gates module)
add_library(gates_lib INTERFACE)
target_include_directories(
  gates_lib
  INTERFACE
    "${GATES_DIR}/include"
)
target_link_libraries(
  gates_lib
  INTERFACE
    Boost::headers
    Eigen3::Eigen
    autodiff::autodiff
)

# 1. Build DAG module
add_library(dag_lib STATIC ${DAG_SOURCES})
target_include_directories(
  dag_lib
  PUBLIC
    "${DAG_DIR}/include"
    "${SABRE_DIR}/include"
    "${GATES_DIR}/include"
)
target_link_libraries(
  dag_lib
  PUBLIC
    gates_lib
    Boost::headers
)
if (BUILD_WITH_GRAPHVIZ)
    target_link_libraries(dag_lib PRIVATE gvc cgraph)
endif()



# 2. Build Qarser module
add_library(qarser_lib STATIC ${QARSER_SOURCES})
target_include_directories(
  qarser_lib
  PUBLIC
    "${QARSER_DIR}/include"
    "${DAG_DIR}/include"
    "${SABRE_DIR}/include" 
    "${GATES_DIR}/include"
)
target_link_libraries(
  qarser_lib
  PUBLIC
    dag_lib
    gates_lib
    Boost::headers
)

# 3. Build SABRE module
add_library(sabre_lib STATIC ${SABRE_SOURCES})
target_include_directories(
  sabre_lib 
  PUBLIC 
    "${SABRE_DIR}/include"
    "${DAG_DIR}/include"
    "${GATES_DIR}/include"
)    
target_link_libraries(
  sabre_lib
  PUBLIC
    dag_lib
    gates_lib
    Boost::headers
)
if (BUILD_WITH_GRAPHVIZ)
    target_link_libraries(sabre_lib PRIVATE gvc cgraph)
endif()
# target_compile_definitions(sabre_test PRIVATE BOOST_NO_CXX98_FUNCTION_BASE)


if(SKBUILD)
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c
            "import pybind11; print(pybind11.get_cmake_dir())"
    OUTPUT_VARIABLE _tmp_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT
  )
  list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")

  set(PYBIND11_FINDPYTHON ON)
  find_package(pybind11 REQUIRED CONFIG)
else()
  ## --  Build Testing -- ##
  set(PYBIND11_FINDPYTHON ON)
  add_subdirectory(extern/pybind11)

  add_executable(
    qsteedcpp_test
      # "${QSTEED_CPP_DIR}/test.cpp"
      "${QARSER_DIR}/src/test.cpp"

  )
  target_link_libraries(
    qsteedcpp_test PRIVATE
      gates_lib
      dag_lib
      qarser_lib
      sabre_lib
  )
endif()

# message(STATUS "Python Interpreter: ${Python_EXECUTABLE}")
# message(STATUS "Python Version: ${Python_VERSION}")


set(target qsteedcpp)
pybind11_add_module(
  ${target}
    ${QARSER_SOURCES}
    ${SABRE_SOURCES}
    "${QSTEED_CPP_DIR}/binding.cpp"
)
target_include_directories(
  ${target}
  PUBLIC
    "${QARSER_DIR}/include"
    "${SABRE_DIR}/include"
    "${DAG_DIR}/include"
    "${GATES_DIR}/include"
)
target_link_libraries(
  ${target} 
  PRIVATE 
    gates_lib
    dag_lib
    sabre_lib 
    qarser_lib
    pybind11::headers
)
if (BUILD_WITH_GRAPHVIZ)
  target_link_libraries(${target} PRIVATE gvc cgraph)
endif()

install(TARGETS ${target} LIBRARY DESTINATION .)


## -- Module Testing -- ##
option(BUILD_TESTS "Build test programs" ON)
message(STATUS "BUILD_TESTS is set to ${BUILD_TESTS}")

if(BUILD_TESTS)
    enable_testing()
    
    # 使用 FetchContent 管理 Google Test
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    # Test Modules
    add_subdirectory(qsteedcpp/tests)
    # add_subdirectory(Qarser/tests)
    # add_subdirectory(DAG/tests)
    # add_subdirectory(SABRE/tests)
endif()
