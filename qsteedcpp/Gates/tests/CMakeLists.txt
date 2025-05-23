
add_executable(
    gates_test
    matrix_test.cpp
    gates_test.cpp
)

target_link_libraries(gates_test
    PRIVATE
    GTest::gtest_main
)

target_include_directories(gates_test
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)


add_test(NAME gates_test COMMAND gates_test) 