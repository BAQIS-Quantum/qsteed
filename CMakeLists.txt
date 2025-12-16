## Project Setting
cmake_minimum_required(VERSION 3.15...3.29)
project(qsteed LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Build type not set - defaulting to Release")
  set(CMAKE_BUILD_TYPE Release)
endif()

# Add conda environment paths
if(DEFINED ENV{CONDA_PREFIX})
  message(STATUS "Detected conda env: $ENV{CONDA_PREFIX}")
  list(APPEND CMAKE_PREFIX_PATH $ENV{CONDA_PREFIX})
  list(APPEND CMAKE_PREFIX_PATH "$ENV{CONDA_PREFIX}/Library")
endif()

if (MSVC)
  add_compile_options(/wd4244) # Disable warning C4244 (Conversion from 'int' to 'float', possible loss of data)
  add_compile_options(/wd4819) # Disable warning C4819 (Code page in Unicode build)
  add_definitions(-D_USE_MATH_DEFINES)
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
set(QUANTUM_CIRCUIT_DIR "${QSTEED_CPP_DIR}/QuantumCircuit")
set(QARSER_DIR "${QSTEED_CPP_DIR}/Qarser")
set(PASSES_DIR "${QSTEED_CPP_DIR}/Passes")

file(GLOB_RECURSE QUANTUM_CIRCUIT_SOURCES "${QUANTUM_CIRCUIT_DIR}/*.cpp")
file(GLOB_RECURSE QARSER_SOURCES "${QARSER_DIR}/*.cpp")
file(GLOB_RECURSE PASSES_SOURCES "${PASSES_DIR}/*.cpp")

include_directories(
  "${QSTEED_CPP_DIR}"
)

# 1. Build QuantumCircuit module
add_library(quantum_circuit_lib STATIC ${QUANTUM_CIRCUIT_SOURCES})
target_include_directories( quantum_circuit_lib PUBLIC
    "${QUANTUM_CIRCUIT_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/eigen"
)
target_link_libraries( quantum_circuit_lib PUBLIC
  Boost::headers
)
if (BUILD_WITH_GRAPHVIZ)
  target_link_libraries(quantum_circuit_lib PRIVATE gvc cgraph)
endif()


# 2. Build Qarser module
add_library(qarser_lib STATIC ${QARSER_SOURCES})
target_include_directories(
  qarser_lib
  PUBLIC
    "${QUANTUM_CIRCUIT_DIR}"
    "${QARSER_DIR}"
)
target_link_libraries( qarser_lib PUBLIC
    Boost::headers
)


# 3. Build Passes module
add_library(passes_lib STATIC ${PASSES_SOURCES})
target_include_directories( passes_lib PUBLIC
    "${QUANTUM_CIRCUIT_DIR}"
    "${PASSES_DIR}"
)
target_link_libraries( passes_lib PUBLIC
    quantum_circuit_lib
    Boost::headers
)


# Find python and pybind11 FIRST (before Torch to avoid Python finding issues)
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


# 4. Optional Torch Backend (AFTER pybind11 to avoid Python finding conflicts)
# Auto-detect torch if available (for pip install qsteed[torch])
option(BUILD_TORCH_BACKEND "Build Torch backend" OFF)

# For non-SKBUILD mode, add extern/libtorch to search path
if(NOT SKBUILD AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/libtorch")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/extern/libtorch")
endif()

# Try to find Torch (will succeed if user did pip install qsteed[torch])
find_package(Torch QUIET)

# Check if torch is actually importable by the Python in the build environment
execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "import torch"
    RESULT_VARIABLE _python_can_import_torch
    OUTPUT_QUIET
    ERROR_QUIET
)

if(BUILD_TORCH_BACKEND OR (Torch_FOUND AND _python_can_import_torch EQUAL 0))
    if(Torch_FOUND AND _python_can_import_torch EQUAL 0)
        message(STATUS "=== Torch detected and importable, building Torch Backend ===")
        message(STATUS "  - Torch version: ${Torch_VERSION}")
        message(STATUS "  - Torch libraries: ${TORCH_LIBRARIES}")

        set(BUILD_TORCH_BACKEND ON)  # Set BEFORE add_subdirectory
        add_subdirectory(qsteedcpp/backends)
        set(TORCH_BACKEND_BINDINGS "${QSTEED_CPP_DIR}/backends/torch/bindings.cpp")
    else()
        if(BUILD_TORCH_BACKEND)
            message(FATAL_ERROR "BUILD_TORCH_BACKEND=ON but torch library not found or not importable by Python build environment. Please ensure PyTorch is installed (e.g., 'pip install torch').")
        else()
            # This case happens if CMake finds Torch but the build Python can't import it.
            # We silently skip the backend to avoid build errors.
            message(STATUS "=== Torch found by CMake, but not importable by Python build environment. Skipping backend. ===")
            message(STATUS "  To enable, ensure 'torch' is in your build environment.")
            set(TORCH_BACKEND_BINDINGS "")
        endif()
    endif()
else()
    message(STATUS "=== Torch not found or not importable, skipping Torch Backend ===")
    message(STATUS "  To enable: pip install qsteed[torch]")
    set(TORCH_BACKEND_BINDINGS "")
endif()


set(target qsteedcpp)
pybind11_add_module( ${target}
  "${QSTEED_CPP_DIR}/bindings/binding.cpp"
  "${QSTEED_CPP_DIR}/bindings/gate_bindings.cpp"
  "${QSTEED_CPP_DIR}/bindings/circuit_bindings.cpp"
  "${QSTEED_CPP_DIR}/bindings/pass_bindings.cpp"
  "${QSTEED_CPP_DIR}/bindings/expression_bindings.cpp"
  ${TORCH_BACKEND_BINDINGS}
)
target_include_directories( ${target} PUBLIC
  "${QUANTUM_CIRCUIT_DIR}"
  "${PASSES_DIR}"
  "${QARSER_DIR}"
)
target_link_libraries( ${target}  PRIVATE
  quantum_circuit_lib
  qarser_lib
  passes_lib
  pybind11::headers
)

# Link torch backend if enabled
if(BUILD_TORCH_BACKEND)
  target_link_libraries(${target} PRIVATE torch_backend_lib "${TORCH_LIBRARIES}")
  target_include_directories(${target} PRIVATE
    "${QSTEED_CPP_DIR}/backends/torch"
    "${TORCH_INCLUDE_DIRS}")
  target_compile_definitions(${target} PRIVATE BUILD_TORCH_BACKEND)

  # Set RPATH to find LibTorch libraries at runtime
  set_target_properties(${target} PROPERTIES
    BUILD_WITH_INSTALL_RPATH FALSE
    INSTALL_RPATH_USE_LINK_PATH TRUE
  )
endif()

install(TARGETS ${target} LIBRARY DESTINATION qsteed)

# Generate Stub
if(SKBUILD)
    message(STATUS "=== SKBUILD mode detected, will generate stub files ===")
    message(STATUS "Python executable: ${PYTHON_EXECUTABLE}")
    message(STATUS "Target: ${target}")
    message(STATUS "Stub output dir: ${CMAKE_CURRENT_BINARY_DIR}/stubs/")

    set(STUB_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/stubs/${target}.pyi")
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/stubs"
        COMMAND ${PYTHON_EXECUTABLE} -m pybind11_stubgen ${target} -o ${CMAKE_CURRENT_BINARY_DIR}/stubs/
        COMMAND ${CMAKE_COMMAND} -E echo "✓ Stub file generated: ${STUB_OUTPUT_FILE}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating type stubs for ${target} module"
        VERBATIM
    )

    # Install the generated .pyi stub file alongside the .so file
    set(STUB_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/qsteed/${target}.pyi")
    install(FILES ${STUB_OUTPUT_FILE}
            DESTINATION qsteed
            OPTIONAL
    )

    # Print installation info at install time
    install(CODE "message(STATUS \"✓ Installing stub file: ${STUB_OUTPUT_FILE}\")")
    install(CODE "message(STATUS \"  to: ${STUB_INSTALL_PATH}\")")

    message(STATUS "Stub generation and install configured")
    message(STATUS "Stub file will be installed to: ${STUB_INSTALL_PATH}")
    message(STATUS "  Generated stub: ${STUB_OUTPUT_FILE}")
    message(STATUS "  Install destination: qsteed/ (relative to CMAKE_INSTALL_PREFIX)")
else()
    message(STATUS "=== NOT in SKBUILD mode, skipping stub generation ===")
endif()


## -- Module Testing -- ##
if(SKBUILD)
    option(BUILD_CPP_TESTS "Build C++ test programs" OFF)
else()
    option(BUILD_CPP_TESTS "Build C++ test programs" ON)
endif()

message(STATUS "BUILD_CPP_TESTS is set to ${BUILD_CPP_TESTS}")

if(BUILD_CPP_TESTS)
    message(STATUS "Building C++ tests...")
    enable_testing()
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    # Test Modules
    add_subdirectory(qsteedcpp/tests)
else()
    message(STATUS "Skipping C++ tests.")
endif()