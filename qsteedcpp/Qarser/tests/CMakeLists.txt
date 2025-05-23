# 添加测试可执行文件
add_executable(qarser_test
    qarser_test.cpp
)

# 链接 Google Test 和项目库
target_link_libraries(qarser_test
    PRIVATE
    GTest::gtest_main
    qsteedcpp
)

# 添加到 CTest
add_test(NAME qarser_test COMMAND qarser_test) 