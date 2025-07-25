#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "circuit/circuit_drawer.h"
#include "circuit/circuit_instruction.h"
#include "gates/base_gate.h"

namespace qsteedcpp {

// 计算 UTF-8 字符串的显示宽度
static size_t utf8_display_width(const std::string& str) {
    size_t width = 0;
    size_t i = 0;
    
    while (i < str.length()) {
        unsigned char c = str[i];
        
        // ASCII 字符
        if (c < 0x80) {
            width++;
            i++;
        }
        // 多字节 UTF-8 字符
        else {
            // 获取 UTF-8 字符的字节数
            int char_len = 0;
            if ((c & 0xE0) == 0xC0) char_len = 2;
            else if ((c & 0xF0) == 0xE0) char_len = 3;
            else if ((c & 0xF8) == 0xF0) char_len = 4;
            else { i++; continue; } // 无效字符，跳过
            
            // 对于大多数符号和希腊字母，假设宽度为 1
            // 对于中文、日文、韩文等，宽度为 2
            // 这里简化处理：假设希腊字母等数学符号宽度为 1
            width++;
            i += char_len;
        }
    }
    
    return width;
}

std::string CircuitDrawer::draw() {
    initialize_grid();
    draw_wires();
    
    for (const auto& inst : instructions_) {
        draw_instruction(inst);
    }
    
    return grid_to_string();
}

void CircuitDrawer::initialize_grid() {
    // 布局：每个量子比特占2行（本身+下面的空行），最后一个量子比特后不需要空行
    int total_rows = num_qubits_ * 2 - 1;
    if (num_clbits_ > 0 && config_.show_clbits) {
        total_rows += 2;  // 空行 + 经典寄存器线
    }
    
    // 初始化网格
    grid_.clear();
    grid_.resize(total_rows);
    column_widths_.clear();
    
    // 设置初始列（标签）
    for (int i = 0; i < num_qubits_; ++i) {
        int row = i * 2;  // 每个量子比特在偶数行
        grid_[row].push_back("q[" + std::to_string(i) + "]: ");
        
        // 空行（除了最后一个量子比特）
        if (i < num_qubits_ - 1) {
            grid_[row + 1].push_back("      ");
        }
    }
    
    if (num_clbits_ > 0 && config_.show_clbits) {
        int clbit_start_row = num_qubits_ * 2 - 1;
        grid_[clbit_start_row].push_back("      ");  // 空行
        grid_[clbit_start_row + 1].push_back("c: " + std::to_string(num_clbits_) + "/");
    }
    
    current_col_ = 1;
}

void CircuitDrawer::draw_wires() {
    // 初始线段（增加一列以对齐）
    int col = allocate_columns(2);
    
    for (int i = 0; i < num_qubits_; ++i) {
        int row = i * 2;
        grid_[row][col] = config_.h_wire;
        grid_[row][col + 1] = config_.h_wire;
        
        // 空行保持空白
        if (i < num_qubits_ - 1) {
            grid_[row + 1][col] = " ";
            grid_[row + 1][col + 1] = " ";
        }
    }
    
    if (num_clbits_ > 0 && config_.show_clbits) {
        int clbit_start_row = num_qubits_ * 2 - 1;
        grid_[clbit_start_row][col] = " ";
        grid_[clbit_start_row][col + 1] = " ";
        grid_[clbit_start_row + 1][col] = config_.double_h_wire;
        grid_[clbit_start_row + 1][col + 1] = config_.double_h_wire;
    }
}

void CircuitDrawer::draw_instruction(const CircuitInstruction& inst) {
    if (inst.is_measurement()) {
        // 检查是单个测量还是批量测量
        const auto& meas = std::get<Measurement>(inst.operation);
        if (meas.size() == 1) {
            draw_measurement(meas.qubit_indices[0], meas.clbit_indices[0]);
        } else {
            // 批量测量：为每个测量分别绘制
            for (size_t i = 0; i < meas.size(); ++i) {
                draw_measurement(meas.qubit_indices[i], meas.clbit_indices[i]);
            }
        }
    } else if (inst.is_barrier()) {
        draw_barrier(inst.qubits);
    } else if (inst.is_gate()) {
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        std::string gate_name = gate->get_name();
        
        if (inst.qubits.size() == 1) {
            draw_single_qubit_gate(gate_name, inst.qubits[0]);
        } else if (inst.qubits.size() == 2) {
            draw_two_qubit_gate(gate_name, inst.qubits, gate.get());
        } else if (inst.qubits.size() == 3) {
            draw_three_qubit_gate(gate_name, inst.qubits);
        }
    }
}

void CircuitDrawer::draw_single_qubit_gate(const std::string& gate_name, int qubit) {
    std::string symbol = get_gate_symbol(gate_name);
    
    // 分配5列用于门：─[H]─
    int col = allocate_columns(5);
    int row = qubit * 2;
    
    // 绘制门
    grid_[row][col] = config_.h_wire;
    grid_[row][col + 1] = "[";
    grid_[row][col + 2] = symbol;
    grid_[row][col + 3] = "]";
    grid_[row][col + 4] = config_.h_wire;
    
    // 其他量子线继续
    for (int i = 0; i < num_qubits_; ++i) {
        if (i != qubit) {
            int other_row = i * 2;
            for (int j = 0; j < 5; ++j) {
                grid_[other_row][col + j] = config_.h_wire;
            }
        }
        
        // 空行保持空白
        if (i < num_qubits_ - 1) {
            for (int j = 0; j < 5; ++j) {
                grid_[i * 2 + 1][col + j] = " ";
            }
        }
    }
    
    // 经典线继续
    if (num_clbits_ > 0 && config_.show_clbits) {
        int clbit_start_row = num_qubits_ * 2 - 1;
        for (int j = 0; j < 5; ++j) {
            grid_[clbit_start_row][col + j] = " ";
            grid_[clbit_start_row + 1][col + j] = config_.double_h_wire;
        }
    }
}

void CircuitDrawer::draw_two_qubit_gate(const std::string& gate_name, const std::vector<int>& qubits, const Gate* gate) {
    int q0 = qubits[0];
    int q1 = qubits[1];
    int min_q = std::min(q0, q1);
    int max_q = std::max(q0, q1);
    
    int col = allocate_columns(5);
    
    if (gate_name == "CX" || gate_name == "CNOT" || gate_name == "cx" || gate_name == "cnot") {
        // 控制点
        int ctrl_row = q0 * 2;
        grid_[ctrl_row][col] = config_.h_wire;
        grid_[ctrl_row][col + 1] = config_.h_wire;
        grid_[ctrl_row][col + 2] = config_.control_dot;
        grid_[ctrl_row][col + 3] = config_.h_wire;
        grid_[ctrl_row][col + 4] = config_.h_wire;
        
        // 目标点
        int targ_row = q1 * 2;
        grid_[targ_row][col] = config_.h_wire;
        grid_[targ_row][col + 1] = "[";
        grid_[targ_row][col + 2] = "X";
        grid_[targ_row][col + 3] = "]";
        grid_[targ_row][col + 4] = config_.h_wire;
        
        // 连接线（在空行中）
        for (int i = min_q; i < max_q; ++i) {
            int conn_row = i * 2 + 1;
            grid_[conn_row][col] = " ";
            grid_[conn_row][col + 1] = " ";
            grid_[conn_row][col + 2] = config_.v_wire;
            grid_[conn_row][col + 3] = " ";
            grid_[conn_row][col + 4] = " ";
        }
        
        // 其他量子线继续
        for (int i = 0; i < num_qubits_; ++i) {
            if (i != q0 && i != q1) {
                int other_row = i * 2;
                for (int j = 0; j < 5; ++j) {
                    grid_[other_row][col + j] = config_.h_wire;
                }
            }
            
            // 不在连接范围内的空行
            if (i < num_qubits_ - 1 && (i < min_q || i >= max_q)) {
                for (int j = 0; j < 5; ++j) {
                    grid_[i * 2 + 1][col + j] = " ";
                }
            }
        }
    } else if (gate_name == "CZ" || gate_name == "cz") {
        // 两个控制点
        for (int q : {q0, q1}) {
            int row = q * 2;
            grid_[row][col] = config_.h_wire;
            grid_[row][col + 1] = config_.h_wire;
            grid_[row][col + 2] = config_.control_dot;
            grid_[row][col + 3] = config_.h_wire;
            grid_[row][col + 4] = config_.h_wire;
        }
        
        // 连接线
        for (int i = min_q; i < max_q; ++i) {
            int conn_row = i * 2 + 1;
            grid_[conn_row][col] = " ";
            grid_[conn_row][col + 1] = " ";
            grid_[conn_row][col + 2] = config_.v_wire;
            grid_[conn_row][col + 3] = " ";
            grid_[conn_row][col + 4] = " ";
        }
        
        // 其他线继续
        for (int i = 0; i < num_qubits_; ++i) {
            if (i != q0 && i != q1) {
                int other_row = i * 2;
                for (int j = 0; j < 5; ++j) {
                    grid_[other_row][col + j] = config_.h_wire;
                }
            }
            
            // 不在连接范围内的空行
            if (i < num_qubits_ - 1 && (i < min_q || i >= max_q)) {
                for (int j = 0; j < 5; ++j) {
                    grid_[i * 2 + 1][col + j] = " ";
                }
            }
        }
    } else if (gate_name == "SWAP" || gate_name == "swap") {
        // SWAP门
        for (int q : {q0, q1}) {
            int row = q * 2;
            grid_[row][col] = config_.h_wire;
            grid_[row][col + 1] = config_.h_wire;
            grid_[row][col + 2] = "×";
            grid_[row][col + 3] = config_.h_wire;
            grid_[row][col + 4] = config_.h_wire;
        }
        
        // 中间经过的量子比特添加交叉点
        for (int i = min_q + 1; i < max_q; ++i) {
            if (i != q0 && i != q1) {
                int row = i * 2;
                grid_[row][col] = config_.h_wire;
                grid_[row][col + 1] = config_.h_wire;
                grid_[row][col + 2] = config_.cross;  // 交叉点
                grid_[row][col + 3] = config_.h_wire;
                grid_[row][col + 4] = config_.h_wire;
            }
        }
        
        // 连接线
        for (int i = min_q; i < max_q; ++i) {
            int conn_row = i * 2 + 1;
            grid_[conn_row][col] = " ";
            grid_[conn_row][col + 1] = " ";
            grid_[conn_row][col + 2] = config_.v_wire;
            grid_[conn_row][col + 3] = " ";
            grid_[conn_row][col + 4] = " ";
        }
        
        // 其他线继续（排除已经处理的中间量子比特）
        for (int i = 0; i < num_qubits_; ++i) {
            if (i != q0 && i != q1 && (i < min_q || i > max_q)) {
                int other_row = i * 2;
                for (int j = 0; j < 5; ++j) {
                    grid_[other_row][col + j] = config_.h_wire;
                }
            }
            
            // 不在连接范围内的空行
            if (i < num_qubits_ - 1 && (i < min_q || i >= max_q)) {
                for (int j = 0; j < 5; ++j) {
                    grid_[i * 2 + 1][col + j] = " ";
                }
            }
        }
    } else {
        // 其他两量子比特门（如 RZZ, RXX, RYY 等）
        std::string gate_label = gate_name;
        if (gate && gate->has_parameters() && gate->parameter_count() > 0) {
            const Parameter& param = gate->get_parameter(0);
            std::stringstream ss;
            ss << gate_name << "(";
            
            // 检查是否有变量
            auto var_names = param.get_variables();
            if (!var_names.empty()) {
                // 有变量，显示 "变量名:值" 格式
                ss << var_names[0] << ":";
                ss << std::fixed << std::setprecision(2) << param.value();
            } else {
                // 没有变量，只显示数值
                ss << std::fixed << std::setprecision(2) << param.value();
            }
            
            ss << ")";
            gate_label = ss.str();
        }
        
        // 计算需要的列数，确保有足够空间显示完整标签
        int label_width = utf8_display_width(gate_label);
        int total_cols = std::max(label_width + 4, 5);  // 增加边距
        
        // 分配足够的列
        col = allocate_columns(total_cols);
        
        // 两个量子比特上的连接点
        for (int q : {q0, q1}) {
            int row = q * 2;
            grid_[row][col] = config_.h_wire;
            grid_[row][col + 1] = config_.h_wire;
            grid_[row][col + 2] = "●";  // 连接点
            for (int j = 3; j < total_cols - 2; ++j) {
                grid_[row][col + j] = config_.h_wire;
            }
            grid_[row][col + total_cols - 2] = config_.h_wire;
            grid_[row][col + total_cols - 1] = config_.h_wire;
        }
        
        // 为中间经过的量子比特添加交叉点
        for (int i = min_q + 1; i < max_q; ++i) {
            if (i != q0 && i != q1) {
                int row = i * 2;
                grid_[row][col] = config_.h_wire;
                grid_[row][col + 1] = config_.h_wire;
                grid_[row][col + 2] = config_.cross;  // 交叉点
                for (int j = 3; j < total_cols - 1; ++j) {
                    grid_[row][col + j] = config_.h_wire;
                }
                grid_[row][col + total_cols - 1] = config_.h_wire;
            }
        }
        
        // 连接线和门标签
        int middle_row = (min_q + max_q) / 2;
        for (int i = min_q; i < max_q; ++i) {
            int conn_row = i * 2 + 1;
            
            if (i == middle_row && max_q - min_q > 0) {
                // 在中间位置显示门标签，紧贴连接线
                int start_pos = 3;  // 直接从位置3开始，紧贴连接线
                
                // 简单的ASCII字符处理，避免Unicode问题
                for (int j = 0; j < total_cols; ++j) {
                    if (j >= start_pos && j - start_pos < static_cast<int>(gate_label.length())) {
                        // 替换非ASCII字符为 '?'
                        char c = gate_label[j - start_pos];
                        if (c < 0 || c > 127) {
                            grid_[conn_row][col + j] = "?";
                        } else {
                            grid_[conn_row][col + j] = std::string(1, c);
                        }
                    } else if (j == 2) {
                        grid_[conn_row][col + j] = config_.v_wire;
                    } else {
                        grid_[conn_row][col + j] = " ";
                    }
                }
            } else {
                // 普通连接线
                for (int j = 0; j < total_cols; ++j) {
                    if (j == 2) {
                        grid_[conn_row][col + j] = config_.v_wire;
                    } else {
                        grid_[conn_row][col + j] = " ";
                    }
                }
            }
        }
        
        // 其他线继续（排除已经处理过的中间量子比特）
        for (int i = 0; i < num_qubits_; ++i) {
            if (i != q0 && i != q1 && (i < min_q || i > max_q)) {
                int other_row = i * 2;
                for (int j = 0; j < total_cols; ++j) {
                    grid_[other_row][col + j] = config_.h_wire;
                }
            }
            
            // 不在连接范围内的空行
            if (i < num_qubits_ - 1 && (i < min_q || i >= max_q)) {
                for (int j = 0; j < total_cols; ++j) {
                    grid_[i * 2 + 1][col + j] = " ";
                }
            }
        }
        
        // 经典线继续
        if (num_clbits_ > 0 && config_.show_clbits) {
            int clbit_start_row = num_qubits_ * 2 - 1;
            for (int j = 0; j < total_cols; ++j) {
                grid_[clbit_start_row][col + j] = " ";
                grid_[clbit_start_row + 1][col + j] = config_.double_h_wire;
            }
        }
        
        return;  // 提前返回
    }
    
    // 经典线继续
    if (num_clbits_ > 0 && config_.show_clbits) {
        int clbit_start_row = num_qubits_ * 2 - 1;
        for (int j = 0; j < 5; ++j) {
            grid_[clbit_start_row][col + j] = " ";
            grid_[clbit_start_row + 1][col + j] = config_.double_h_wire;
        }
    }
}

void CircuitDrawer::draw_three_qubit_gate(const std::string& gate_name, const std::vector<int>& qubits) {
    int col = allocate_columns(5);
    
    if (gate_name == "CCX" || gate_name == "Toffoli" || gate_name == "ccx") {
        int q0 = qubits[0];
        int q1 = qubits[1];
        int q2 = qubits[2];
        int min_q = std::min({q0, q1, q2});
        int max_q = std::max({q0, q1, q2});
        
        // 两个控制点
        for (int q : {q0, q1}) {
            int row = q * 2;
            grid_[row][col] = config_.h_wire;
            grid_[row][col + 1] = config_.h_wire;
            grid_[row][col + 2] = config_.control_dot;
            grid_[row][col + 3] = config_.h_wire;
            grid_[row][col + 4] = config_.h_wire;
        }
        
        // 目标点
        int targ_row = q2 * 2;
        grid_[targ_row][col] = config_.h_wire;
        grid_[targ_row][col + 1] = "[";
        grid_[targ_row][col + 2] = "X";
        grid_[targ_row][col + 3] = "]";
        grid_[targ_row][col + 4] = config_.h_wire;
        
        // 连接线
        for (int i = min_q; i < max_q; ++i) {
            int conn_row = i * 2 + 1;
            grid_[conn_row][col] = " ";
            grid_[conn_row][col + 1] = " ";
            grid_[conn_row][col + 2] = config_.v_wire;
            grid_[conn_row][col + 3] = " ";
            grid_[conn_row][col + 4] = " ";
        }
        
        // 其他线继续
        for (int i = 0; i < num_qubits_; ++i) {
            if (i != q0 && i != q1 && i != q2) {
                int other_row = i * 2;
                for (int j = 0; j < 5; ++j) {
                    grid_[other_row][col + j] = config_.h_wire;
                }
            }
            
            // 不在连接范围内的空行
            if (i < num_qubits_ - 1 && (i < min_q || i >= max_q)) {
                for (int j = 0; j < 5; ++j) {
                    grid_[i * 2 + 1][col + j] = " ";
                }
            }
        }
    }
    
    // 经典线继续
    if (num_clbits_ > 0 && config_.show_clbits) {
        int clbit_start_row = num_qubits_ * 2 - 1;
        for (int j = 0; j < 5; ++j) {
            grid_[clbit_start_row][col + j] = " ";
            grid_[clbit_start_row + 1][col + j] = config_.double_h_wire;
        }
    }
}

void CircuitDrawer::draw_measurement(int qubit, int clbit) {
    // 测量符号
    int col = allocate_columns(5);
    int row = qubit * 2;
    
    // 测量门
    grid_[row][col] = config_.h_wire;
    grid_[row][col + 1] = "[";
    grid_[row][col + 2] = "M";
    grid_[row][col + 3] = "]";
    grid_[row][col + 4] = config_.h_wire;
    
    // 其他量子线继续，但在测量列显示双垂直线
    for (int i = 0; i < num_qubits_; ++i) {
        if (i != qubit) {
            int other_row = i * 2;
            grid_[other_row][col] = config_.h_wire;
            grid_[other_row][col + 1] = config_.h_wire;
            grid_[other_row][col + 2] = config_.double_v_wire;  // 所有量子比特都显示双垂直线
            grid_[other_row][col + 3] = config_.h_wire;
            grid_[other_row][col + 4] = config_.h_wire;
        }
        
        // 空行也需要双垂直线
        if (i < num_qubits_ - 1) {
            grid_[i * 2 + 1][col] = " ";
            grid_[i * 2 + 1][col + 1] = " ";
            grid_[i * 2 + 1][col + 2] = config_.double_v_wire;  // 空行中的双垂直线
            grid_[i * 2 + 1][col + 3] = " ";
            grid_[i * 2 + 1][col + 4] = " ";
        }
    }
    
    // 连接到经典比特
    if (num_clbits_ > 0 && config_.show_clbits) {
        int clbit_start_row = num_qubits_ * 2 - 1;
        
        // 双垂直线已经在上面的循环中设置好了，不需要额外处理
        
        // 经典寄存器线
        grid_[clbit_start_row + 1][col] = config_.double_h_wire;
        grid_[clbit_start_row + 1][col + 1] = config_.double_h_wire;
        grid_[clbit_start_row + 1][col + 2] = config_.double_cross_up;
        grid_[clbit_start_row + 1][col + 3] = config_.double_h_wire;
        grid_[clbit_start_row + 1][col + 4] = config_.double_h_wire;
        
        // 在垂直线下方直接添加经典比特索引
        int idx_col = allocate_columns(1);
        grid_[clbit_start_row][idx_col] = " ";
        grid_[clbit_start_row + 1][idx_col] = " ";
        
        // 添加经典比特索引数字（直接在交叉点下方）
        int num_col = allocate_columns(1);
        grid_[clbit_start_row][num_col] = " ";
        grid_[clbit_start_row + 1][num_col] = std::to_string(clbit);
        
        // 所有量子线继续
        for (int i = 0; i < num_qubits_; ++i) {
            int other_row = i * 2;
            grid_[other_row][idx_col] = config_.h_wire;
            grid_[other_row][num_col] = config_.h_wire;
            
            if (i < num_qubits_ - 1) {
                grid_[i * 2 + 1][idx_col] = " ";
                grid_[i * 2 + 1][num_col] = " ";
            }
        }
    }
}

void CircuitDrawer::draw_barrier(const std::vector<int>& qubits) {
    int col = allocate_columns(3);
    
    if (qubits.empty()) {
        // 全局屏障
        for (int i = 0; i < num_qubits_; ++i) {
            int row = i * 2;
            grid_[row][col] = " ";
            grid_[row][col + 1] = "░";
            grid_[row][col + 2] = " ";
            
            // 空行
            if (i < num_qubits_ - 1) {
                for (int j = 0; j < 3; ++j) {
                    grid_[row + 1][col + j] = " ";
                }
            }
        }
    } else {
        // 部分屏障
        for (int i = 0; i < num_qubits_; ++i) {
            int row = i * 2;
            bool has_barrier = std::find(qubits.begin(), qubits.end(), i) != qubits.end();
            
            if (has_barrier) {
                grid_[row][col] = " ";
                grid_[row][col + 1] = "░";
                grid_[row][col + 2] = " ";
            } else {
                grid_[row][col] = config_.h_wire;
                grid_[row][col + 1] = config_.h_wire;
                grid_[row][col + 2] = config_.h_wire;
            }
            
            // 空行
            if (i < num_qubits_ - 1) {
                for (int j = 0; j < 3; ++j) {
                    grid_[row + 1][col + j] = " ";
                }
            }
        }
    }
    
    // 经典线继续
    if (num_clbits_ > 0 && config_.show_clbits) {
        int clbit_start_row = num_qubits_ * 2 - 1;
        for (int j = 0; j < 3; ++j) {
            grid_[clbit_start_row][col + j] = " ";
            grid_[clbit_start_row + 1][col + j] = config_.double_h_wire;
        }
    }
}

int CircuitDrawer::allocate_columns(int width) {
    int start_col = current_col_;
    
    // 确保网格有足够的列
    for (auto& row : grid_) {
        int new_size = start_col + width;
        while (row.size() < new_size) {
            row.push_back("");
        }
    }
    
    current_col_ += width;
    return start_col;
}

std::string CircuitDrawer::format_gate_name(const std::string& name, int width) {
    if (static_cast<int>(name.length()) > width) {
        return name.substr(0, width);
    }
    
    int padding = width - name.length();
    int left_pad = padding / 2;
    int right_pad = padding - left_pad;
    
    return std::string(left_pad, ' ') + name + std::string(right_pad, ' ');
}

std::string CircuitDrawer::get_gate_symbol(const std::string& gate_name) const {
    static const std::map<std::string, std::string> gate_symbols = {
        {"I", "I"},
        {"X", "X"},
        {"Y", "Y"}, 
        {"Z", "Z"},
        {"H", "H"},
        {"S", "S"},
        {"T", "T"},
        {"RX", "RX"},
        {"RY", "RY"},
        {"RZ", "RZ"},
        {"CNOT", "X"},
        {"CZ", "Z"},
        {"SWAP", "×"},
        {"CCX", "X"},
        {"Toffoli", "X"},
        {"Sdg", "S†"},
        {"Tdg", "T†"},
        {"iSWAP", "iS"},
        {"P", "P"},
        {"U3", "U3"},
        // 参数化两量子比特门
        {"RZZ", "RZZ"},
        {"RXX", "RXX"},
        {"RYY", "RYY"}
    };
    
    auto it = gate_symbols.find(gate_name);
    if (it != gate_symbols.end()) {
        return it->second;
    }
    
    // 返回门名本身，或默认"?"
    if (!gate_name.empty()) {
        return gate_name;
    }
    return "?";
}

std::string CircuitDrawer::grid_to_string() const {
    std::stringstream ss;
    
    // 输出网格
    for (size_t r = 0; r < grid_.size(); ++r) {
        for (size_t c = 0; c < grid_[r].size(); ++c) {
            ss << grid_[r][c];
        }
        ss << std::endl;
    }
    
    return ss.str();
}

} // namespace qsteedcpp