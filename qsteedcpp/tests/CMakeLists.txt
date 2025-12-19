message("TESTing")

set(QSTEED_CPP_ROOT "${CMAKE_SOURCE_DIR}/qsteedcpp")

# Circuit tests
add_executable(circuit_test
    "circuit/expression_test.cpp"
)
target_link_libraries(circuit_test PRIVATE
    GTest::gtest_main
    qsteed_core
)
target_include_directories(circuit_test PUBLIC
    "${QSTEED_CPP_ROOT}/QuantumCircuit/circuit"
)

# Unroll tests
add_executable(unroll_test
    "unroll/unroll_test.cpp"
    "${QSTEED_CPP_ROOT}/Passes/unroll/decomposition_rules.cpp"
    "${QSTEED_CPP_ROOT}/Passes/unroll/rules/controlled_rotation_rules.cpp"
    "${QSTEED_CPP_ROOT}/Passes/unroll/rules/three_qubit_gate_rules.cpp"
    "${QSTEED_CPP_ROOT}/Passes/unroll/rules/swap_rules.cpp"
    "${QSTEED_CPP_ROOT}/Passes/unroll/rules/two_qubit_rules.cpp"
    "${QSTEED_CPP_ROOT}/Passes/unroll/rules/single_qubit_rules.cpp"
)
target_link_libraries(unroll_test PRIVATE
    GTest::gtest_main
    qsteed_core
)

# SABRE tests
add_executable(sabre_test
    "dag/sabre_test.cpp"
)
target_link_libraries(sabre_test PRIVATE
    GTest::gtest_main
    qsteed_core
    Boost::boost
)

# Qarser tests
add_executable(qarser_test
    "qarser/qarser_test.cpp"
)
target_link_libraries(qarser_test PRIVATE
    GTest::gtest_main
    qsteed_core
)

# Torch Backend tests (only if BUILD_TORCH_BACKEND is enabled)
if(BUILD_TORCH_BACKEND)
    add_executable(torch_backend_test
        "backends/torch_backend_test.cpp"
    )
    target_link_libraries(torch_backend_test PRIVATE
        GTest::gtest_main
        qsteed_core
        # Note: qsteed_core is already linked to torch libraries
    )

    add_test(NAME torch_backend_test COMMAND torch_backend_test)

    # Autograd test
    add_executable(torch_autograd_simple_test
        "backends/torch_autograd_simple_test.cpp"
    )
    target_link_libraries(torch_autograd_simple_test PRIVATE
        GTest::gtest_main
        qsteed_core
    )

    add_test(NAME torch_autograd_simple_test COMMAND torch_autograd_simple_test)
endif()

# Add all tests to CTest
add_test(NAME circuit_test COMMAND circuit_test)
# add_test(NAME parameter_test COMMAND parameter_test) # This test seems to be missing its source
add_test(NAME sabre_test COMMAND sabre_test)
add_test(NAME qarser_test COMMAND qarser_test)
add_test(NAME unroll_test COMMAND unroll_test)