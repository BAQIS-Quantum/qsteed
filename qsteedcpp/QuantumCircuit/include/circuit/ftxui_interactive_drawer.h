#pragma once

#include <string>
#include <vector>
#include "circuit_instruction.h"
#include "ftxui/component/component.hpp"

namespace qsteedcpp {

class FTXUIInteractiveDrawer {
public:
    FTXUIInteractiveDrawer(const std::vector<CircuitInstruction>& instructions, 
                          int num_qubits, int num_clbits)
        : instructions_(instructions), 
          num_qubits_(num_qubits), 
          num_clbits_(num_clbits) {}
    
    // 创建交互式组件
    ftxui::Component create_component();
    
    // 运行交互式界面
    void run();
    
private:
    const std::vector<CircuitInstruction>& instructions_;
    int num_qubits_;
    int num_clbits_;
    
    // 滚动状态 (0.0 到 1.0 之间的相对位置)
    float scroll_x_ = 0.0f;
    float scroll_y_ = 0.0f;
    
    // 生成状态栏
    ftxui::Element render_status_bar();
};

} // namespace qsteedcpp