#include <gtest/gtest.h>
#include <cmath>
#include "QuantumCircuit/expression/expr.h"

using namespace qsteedcpp;

class GateTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

TEST_F(GateTest, ExpressionSystem) {
 
    Parameter p1(0.5, true);   // value=0.5, trainable
    Parameter p2(0.5, false);  // value=0.5, non-trainable
    Parameter p3(1.0, true);   // value=1.0, trainable

    EXPECT_EQ(p1, p2);      // true (值相等)
    EXPECT_FALSE(p1.same(p2));            // false (不是同一个参数对象，UUID 不同)
    EXPECT_LE(p1, p3);      // true (0.5 < 1.0)
}
