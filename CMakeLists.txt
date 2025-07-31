## Project Setting
cmake_minimum_required(VERSION 3.15...3.29)
project(sabre LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用旧的 ABI 以兼容较旧的 libstdc++
# 限制 GLIBCXX 版本
add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")


if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Build type not set - defaulting to Release")
  set(CMAKE_BUILD_TYPE Release)
endif()

# Set CMake module path to include custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

if (MSVC)
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


# 外部依赖管理
include(cmake/Dependencies.cmake)
set(BUILD_TESTING OFF CACHE BOOL "Disable all testing")

# 确保 FTXUI 目标被正确创建
if(NOT TARGET screen)
  message(WARNING "FTXUI screen target not found")
endif()
if(NOT TARGET dom)
  message(WARNING "FTXUI dom target not found")
endif()
if(NOT TARGET component)
  message(WARNING "FTXUI component target not found")
endif()


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
set(QUANTUM_CIRCUIT_DIR "${QSTEED_CPP_DIR}/QuantumCircuit")
set(PASSES_DIR "${QSTEED_CPP_DIR}/passes")

file(GLOB_RECURSE QARSER_SOURCES "${QARSER_DIR}/*.cpp")
file(GLOB_RECURSE SABRE_SOURCES "${SABRE_DIR}/*.cpp")
file(GLOB_RECURSE DAG_SOURCES "${DAG_DIR}/*.cpp")
file(GLOB_RECURSE QUANTUM_CIRCUIT_SOURCES "${QUANTUM_CIRCUIT_DIR}/*.cpp")
file(GLOB_RECURSE PASSES_SOURCES "${PASSES_DIR}/*.cpp")

include_directories(
  "${QSTEED_CPP_DIR}"
)

# 0. Build QuantumCircuit module)
add_library(quantum_circuit_lib STATIC ${QUANTUM_CIRCUIT_SOURCES})
target_include_directories(
  quantum_circuit_lib
  PUBLIC
    "${QUANTUM_CIRCUIT_DIR}/include"
)
target_link_libraries(
  quantum_circuit_lib
  PUBLIC
    Boost::headers
    Eigen3::Eigen
    autodiff::autodiff
    component
    dom
    screen
)

# 1. Build DAG module
add_library(dag_lib STATIC ${DAG_SOURCES})
target_include_directories(
  dag_lib
  PUBLIC
    "${DAG_DIR}/include"
    "${SABRE_DIR}/include"
    "${QUANTUM_CIRCUIT_DIR}/include"
)
target_link_libraries(
  dag_lib
  PUBLIC
    quantum_circuit_lib
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
    "${QUANTUM_CIRCUIT_DIR}/include"
)
target_link_libraries(
  qarser_lib
  PUBLIC
    dag_lib
    quantum_circuit_lib
    Boost::headers
)

# 3. Build SABRE module
add_library(sabre_lib STATIC ${SABRE_SOURCES})
target_include_directories(
  sabre_lib 
  PUBLIC 
    "${SABRE_DIR}/include"
    "${DAG_DIR}/include"
    "${QUANTUM_CIRCUIT_DIR}/include"
)    
target_link_libraries(
  sabre_lib
  PUBLIC
    dag_lib
    quantum_circuit_lib
    Boost::headers
)
if (BUILD_WITH_GRAPHVIZ)
    target_link_libraries(sabre_lib PRIVATE gvc cgraph)
endif()

# 4. Build Passes module
add_library(passes_lib STATIC ${PASSES_SOURCES})
target_include_directories(
  passes_lib
  PUBLIC
    "${PASSES_DIR}"
    "${QUANTUM_CIRCUIT_DIR}/include"
)
target_link_libraries(
  passes_lib
  PUBLIC
    quantum_circuit_lib
    Boost::headers
)


# Find python and pybind11
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
  set(PYBIND11_FINDPYTHON ON)
  add_subdirectory(extern/pybind11)
endif()

message(STATUS "Python Interpreter: ${Python_EXECUTABLE}")
message(STATUS "Python Version: ${Python_VERSION}")


set(target qsteedcpp)
pybind11_add_module(
  ${target}
    ${QARSER_SOURCES}
    ${SABRE_SOURCES}
    ${QUANTUM_CIRCUIT_SOURCES}
    ${PASSES_SOURCES}
    "${QSTEED_CPP_DIR}/bindings/binding.cpp"
    "${QSTEED_CPP_DIR}/bindings/gate_bindings.cpp"
    "${QSTEED_CPP_DIR}/bindings/circuit_bindings.cpp"
    "${QSTEED_CPP_DIR}/bindings/pass_bindings.cpp"
)
target_include_directories(
  ${target}
  PUBLIC
    "${QARSER_DIR}/include"
    "${SABRE_DIR}/include"
    "${DAG_DIR}/include"
    "${QUANTUM_CIRCUIT_DIR}/include"
    "${PASSES_DIR}"
)
target_link_libraries(
  ${target} 
  PRIVATE 
    quantum_circuit_lib
    dag_lib
    sabre_lib 
    qarser_lib
    passes_lib
    pybind11::headers
)
if (BUILD_WITH_GRAPHVIZ)
  target_link_libraries(${target} PRIVATE gvc cgraph)
endif()

install(TARGETS ${target} LIBRARY DESTINATION passes/mapping/cpp_utils)

# Generate Stub
if(SKBUILD)
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${PYTHON_EXECUTABLE} -m pybind11_stubgen ${target} --output-dir ${CMAKE_CURRENT_BINARY_DIR}/stubs
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating type stubs for ${target} module"
        VERBATIM
    )
    
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/stubs/${target}/__init__.pyi 
            DESTINATION passes/mapping/cpp_utils/${target}
            OPTIONAL)
endif()


## -- Module Testing -- ##
option(BUILD_TESTS "Build test programs" ON)
message(STATUS "BUILD_TESTS is set to ${BUILD_TESTS}")

if(BUILD_TESTS)
    enable_testing()
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    # Test Modules
    add_subdirectory(qsteedcpp/tests)
endif()