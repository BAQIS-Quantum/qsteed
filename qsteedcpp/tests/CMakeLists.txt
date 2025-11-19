message("TESTing")

# Circuit tests
add_executable(circuit_test
    "circuit/symbolic_test.cpp"
    "circuit/test_circuit_drawer.cpp"
)

target_link_libraries(circuit_test PRIVATE
    GTest::gtest_main
    quantum_circuit_lib
)

target_include_directories(circuit_test PUBLIC
    "QuantumCircuit/circuit"
)

# Unroll tests
add_executable(unroll_test
    unroll/unroll_test.cpp
    ${QSTEED_CPP_DIR}/Passes/unroll/decomposition_rules.cpp
    ${QSTEED_CPP_DIR}/Passes/unroll/rules/controlled_rotation_rules.cpp
    ${QSTEED_CPP_DIR}/Passes/unroll/rules/three_qubit_gate_rules.cpp
    ${QSTEED_CPP_DIR}/Passes/unroll/rules/swap_rules.cpp
    ${QSTEED_CPP_DIR}/Passes/unroll/rules/two_qubit_rules.cpp
    ${QSTEED_CPP_DIR}/Passes/unroll/rules/single_qubit_rules.cpp
)

target_link_libraries(unroll_test
    PRIVATE
    GTest::gtest_main
    quantum_circuit_lib
    passes_lib
)

target_include_directories(unroll_test PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../QuantumCircuit/
    ${QSTEED_CPP_DIR}/Passes
)



# SABRE tests
add_executable(sabre_test
    "dag/sabre_test.cpp"
)

target_link_libraries(sabre_test PRIVATE
    GTest::gtest_main
    passes_lib
    Boost::boost
)

target_include_directories(sabre_test PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../QuantumCircuit
    ${CMAKE_CURRENT_SOURCE_DIR}/../Passes
)

# Qarser tests
add_executable(qarser_test
    qarser/qarser_test.cpp
)

target_link_libraries(qarser_test PRIVATE
    GTest::gtest_main
    qarser_lib
    quantum_circuit_lib
)

target_include_directories(qarser_test
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../Qarser/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../DAG/include
)

# Add all tests to CTest
add_test(NAME circuit_test COMMAND circuit_test)
add_test(NAME parameter_test COMMAND parameter_test)
add_test(NAME sabre_test COMMAND sabre_test)
add_test(NAME qarser_test COMMAND qarser_test)
add_test(NAME unroll_test COMMAND unroll_test)