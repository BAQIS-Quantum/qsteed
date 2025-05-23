# 添加测试可执行文件
add_executable(sabre_test
    sabre_test.cpp
)

# 链接 Google Test 和项目库
target_link_libraries(sabre_test
    PRIVATE
    GTest::gtest_main
    qsteedcpp
)

# 添加到 CTest
add_test(NAME sabre_test COMMAND sabre_test) 