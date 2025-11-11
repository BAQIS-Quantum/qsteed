#pragma once
#include <string>
#include <vector>
#include "circuit_instruction.h"

namespace qsteedcpp {

struct DrawConfig {
    int wire_spacing = 0;      // 线之间的间距 (0 for compact)
    int gate_width = 5;        // 门的最小宽度
    bool show_clbits = true;   // 是否显示经典比特
    bool unicode = true;       // 使用Unicode字符
    
    // Unicode box drawing characters
    std::string h_wire = "─";           // 水平线
    std::string v_wire = "│";           // 垂直线
    std::string cross = "┼";            // 交叉点
    std::string box_top_left = "┌";     // 左上角
    std::string box_top_right = "┐";    // 右上角
    std::string box_bottom_left = "└";  // 左下角
    std::string box_bottom_right = "┘"; // 右下角
    std::string box_t_down = "┬";       // T型向下
    std::string box_t_up = "┴";         // T型向上
    std::string box_t_right = "├";      // T型向右
    std::string box_t_left = "┤";       // T型向左
    std::string double_h_wire = "═";    // 双横线
    std::string double_v_wire = "║";    // 双竖线
    std::string double_bottom_left = "╚"; // 双线左下
    std::string double_bottom_right = "╝"; // 双线右下
    std::string double_cross_up = "╩";   // 双线上交叉
    std::string control_dot = "■";      // 控制点
    std::string target_x = "┴";         // 目标点(X门)
    std::string measure_top = "┌─┐";    // 测量符号顶部
    std::string measure_mid = "┤M├";    // 测量符号中部
    std::string measure_bot = "└╥┘";    // 测量符号底部
};

class CircuitDrawer {
private:
    const std::vector<CircuitInstruction>& instructions_;
    int num_qubits_;
    int num_clbits_;
    DrawConfig config_;
    
    // 绘图网格
    std::vector<std::vector<std::string>> grid_;
    std::vector<int> column_widths_;
    int current_col_ = 0;
    
public:
    CircuitDrawer(const std::vector<CircuitInstruction>& instructions,
                  int num_qubits, int num_clbits,
                  const DrawConfig& config = DrawConfig())
        : instructions_(instructions), num_qubits_(num_qubits), 
          num_clbits_(num_clbits), config_(config) {}
    
    std::string draw();
    
private:
    void initialize_grid();
    void draw_wires();
    void draw_instruction(const CircuitInstruction& inst);
    void draw_single_qubit_gate(const std::string& gate_name, int qubit);
    void draw_two_qubit_gate(const std::string& gate_name, const std::vector<int>& qubits, const Gate* gate = nullptr);
    void draw_three_qubit_gate(const std::string& gate_name, const std::vector<int>& qubits);
    void draw_measurement(int qubit, int clbit);
    void draw_barrier(const std::vector<int>& qubits);
    
    int allocate_columns(int width);
    std::string format_gate_name(const std::string& name, int width);
    std::string grid_to_string() const;
    
    std::string get_gate_symbol(const std::string& gate_name) const;
    

    int qubit_row(int qubit) const { 
        return qubit * 2; 
    }
    
    int clbit_row(int clbit) const { 
        return num_qubits_ * 2 - 1 + 1; 
    }
};

} // namespace qsteedcpp