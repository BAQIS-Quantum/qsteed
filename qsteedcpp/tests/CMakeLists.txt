add_executable(
    quantum_circuit_test
        matrix_test.cpp
        parameter_test.cpp
        gates_test.cpp
)

target_link_libraries(quantum_circuit_test
    PRIVATE
    GTest::gtest_main
    Eigen3::Eigen
    autodiff::autodiff
)

target_include_directories(quantum_circuit_test
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../QuantumCircuit/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../DAG/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../SABRE/include
)


add_test(NAME quantum_circuit_test COMMAND quantum_circuit_test) 