# 添加测试可执行文件
add_executable(dag_test
    dag_test.cpp
)

# 链接 Google Test 和项目库
target_link_libraries(dag_test
    PRIVATE
    GTest::gtest_main
    qsteedcpp
)

# 添加到 CTest
add_test(NAME dag_test COMMAND dag_test) 