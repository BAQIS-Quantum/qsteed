# This file builds the core C++ shared library, libqsteed_core.so
# It has no knowledge of Python or pybind11.

# File filter
set(QSTEED_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(QUANTUM_CIRCUIT_DIR "${QSTEED_CPP_DIR}/QuantumCircuit")
set(QARSER_DIR "${QSTEED_CPP_DIR}/Qarser")
set(PASSES_DIR "${QSTEED_CPP_DIR}/Passes")

file(GLOB_RECURSE QUANTUM_CIRCUIT_SOURCES "${QUANTUM_CIRCUIT_DIR}/*.cpp")
file(GLOB_RECURSE QARSER_SOURCES "${QARSER_DIR}/*.cpp")
file(GLOB_RECURSE PASSES_SOURCES "${PASSES_DIR}/*.cpp")

# 1. Consolidate core logic into a single SHARED library
add_library(qsteed_core SHARED
  ${QUANTUM_CIRCUIT_SOURCES}
  ${QARSER_SOURCES}
  ${PASSES_SOURCES}
)

# Set common include directories for the core library
target_include_directories(qsteed_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}" # Add the qsteedcpp directory itself to the include path
    "${CMAKE_SOURCE_DIR}/vendor/eigen" # Use CMAKE_SOURCE_DIR to get to project root
)

# Link common dependencies for the core library
target_link_libraries(qsteed_core PUBLIC
  Boost::headers
)

# Link Graphviz if enabled
if (BUILD_WITH_GRAPHVIZ)
  target_link_libraries(qsteed_core PRIVATE gvc cgraph)
endif()

target_precompile_headers(qsteed_core PUBLIC "${QSTEED_CPP_DIR}/pch.h")


# Conditionally build and link the Torch backend
if(BUILD_TORCH_BACKEND)
    add_subdirectory(backends)

    # Link torch libs into the core lib
    target_link_libraries(qsteed_core PRIVATE torch_backend_lib "${TORCH_LIBRARIES}")
    target_include_directories(qsteed_core PRIVATE
        "${QSTEED_CPP_DIR}/backends/torch"
        "${TORCH_INCLUDE_DIRS}"
    )
    target_compile_definitions(qsteed_core PRIVATE BUILD_TORCH_BACKEND)
endif()

# Set a variable in the parent scope to indicate the path to torch binding sources if available
if(BUILD_TORCH_BACKEND AND EXISTS "${QSTEED_CPP_DIR}/backends/torch/bindings.cpp")
    set(TORCH_BINDING_SOURCES "${QSTEED_CPP_DIR}/backends/torch/bindings.cpp" PARENT_SCOPE)
else()
    set(TORCH_BINDING_SOURCES "" PARENT_SCOPE)
endif()
