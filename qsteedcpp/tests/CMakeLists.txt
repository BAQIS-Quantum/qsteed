add_executable(
    gates_test
    matrix_test.cpp
    parameter_test.cpp
)

target_link_libraries(gates_test
    PRIVATE
    GTest::gtest_main
    Eigen3::Eigen
    autodiff::autodiff
)

target_include_directories(gates_test
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../Gates/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../DAG/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../SABRE/include
)


add_test(NAME gates_test COMMAND gates_test) 