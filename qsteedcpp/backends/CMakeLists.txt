# Backends CMakeLists.txt
# Manages all optional backends (torch, etc.)

message(STATUS "Configuring Backends...")

# Torch Backend
if(BUILD_TORCH_BACKEND)
    message(STATUS "  Building Torch Backend...")

    # Collect all torch backend source files
    file(GLOB TORCH_BACKEND_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/torch/tensor_network/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/torch/torch_backend.cpp"
    )

    # Build static library
    add_library(torch_backend_lib STATIC ${TORCH_BACKEND_SOURCES})

    # Include directories
    target_include_directories(torch_backend_lib PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/torch"
        "${TORCH_INCLUDE_DIRS}"
        "${QSTEED_CPP_DIR}/QuantumCircuit"
    )

    # Link libraries
    target_link_libraries(torch_backend_lib PUBLIC
        "${TORCH_LIBRARIES}"
        quantum_circuit_lib
    )

    # Find torch_python library for Python bindings (required for pybind11<->torch conversion)
    # Try multiple possible locations
    find_library(TORCH_PYTHON_LIBRARY torch_python
        PATHS
            "${TORCH_INSTALL_PREFIX}/lib"
            "${Python_SITELIB}/torch/lib"
        NO_DEFAULT_PATH
    )

    if(NOT TORCH_PYTHON_LIBRARY)
        # Fallback: try with lib prefix
        find_library(TORCH_PYTHON_LIBRARY torch_python
            NAMES torch_python libtorch_python
            PATHS
                "${TORCH_INSTALL_PREFIX}/lib"
                "${Python_SITELIB}/torch/lib"
        )
    endif()

    if(TORCH_PYTHON_LIBRARY)
        target_link_libraries(torch_backend_lib PUBLIC "${TORCH_PYTHON_LIBRARY}")
        message(STATUS "    - Found torch_python: ${TORCH_PYTHON_LIBRARY}")
    else()
        message(WARNING "    - torch_python library not found - torch::Tensor bindings may crash!")
        message(STATUS "    - Searched in: ${TORCH_INSTALL_PREFIX}/lib and ${Python_SITELIB}/torch/lib")
    endif()

    message(STATUS "  Torch Backend library configured successfully")
endif()

message(STATUS "Backends configuration complete")
