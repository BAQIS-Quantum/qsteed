#include <iomanip>
#include <sstream>
#include <algorithm>
#include "circuit/ftxui_circuit_drawer.h"
#include "circuit/circuit_instruction.h"
#include "gates/base_gate.h"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"

using namespace ftxui;

namespace qsteedcpp {


// 单量子比特门元素
static Element GateElement(const std::string& label) {
    return text("─[" + label + "]─");
}

// 带参数的门元素
static Element ParameterizedGateElement(const std::string& label, int min_width = 7) {
    int label_width = label.length() + 4; // +4 for "─[" and "]─"
    int width = std::max(min_width, label_width);
    int padding = width - label_width;
    int left_pad = padding / 2;
    int right_pad = padding - left_pad;
    
    std::string result = "─";
    for (int i = 0; i < left_pad; ++i) result += "─";
    result += "[" + label + "]";
    for (int i = 0; i < right_pad; ++i) result += "─";
    result += "─";
    
    return text(result);
}

// 控制点元素
static Element ControlElement() {
    return text("──●──");
}

// 目标门元素（如CNOT的X门）
static Element TargetElement(const std::string& label = "X") {
    return text("─[" + label + "]─");
}

// 垂直连接线
static Element VerticalLine() {
    return text("  │  ");
}

// 交叉点（用于穿过中间量子比特的连接）
static Element CrossLine() {
    return text("──┼──");
}

// SWAP门的交叉符号
static Element SwapElement() {
    return text("──×──");
}

// 普通量子线
static Element QuantumWire(int width = 5) {
    std::string result;
    for (int i = 0; i < width; ++i) result += "─";
    return text(result);
}

// 经典线
static Element ClassicalWire(int width = 5) {
    std::string result;
    for (int i = 0; i < width; ++i) result += "═";
    return text(result);
}

// 空元素
static Element EmptySpace(int width = 5) {
    return text(std::string(width, ' '));
}

// 测量元素
static Element MeasurementElement() {
    return text("─[M]─");
}

// 屏障元素
static Element BarrierElement() {
    return text("  ░  ");
}

// 双线垂直连接（用于测量）
static Element DoubleVerticalLine() {
    return text("  ║  ");
}

// 经典线与双垂直线的交叉
static Element ClassicalCross() {
    return text("══╩══");
}

// 辅助函数：计算指令需要的列宽
static int calculate_column_width(const CircuitInstruction& inst) {
    if (inst.is_gate()) {
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        std::string gate_name = gate->name();
        
        // 计算参数化门的标签
        if (gate->has_parameters() && gate->parameter_count() > 0) {
            const Parameter& param = gate->get_parameter(0);
            std::stringstream ss;
            ss << gate_name << "(";
            
            auto var_names = param.get_variables();
            if (!var_names.empty()) {
                ss << var_names[0] << ":";
                ss << std::fixed << std::setprecision(2) << param.value();
            } else {
                ss << std::fixed << std::setprecision(2) << param.value();
            }
            
            ss << ")";
            std::string gate_label = ss.str();
            
            if (inst.qubits.size() == 2) {
                // 双量子比特参数化门需要额外空间
                return std::max(7, static_cast<int>(gate_label.length()) + 4);
            }
            return std::max(5, static_cast<int>(gate_label.length()) + 4);
        }
        
        if (inst.qubits.size() == 1) {
            return std::max(5, static_cast<int>(gate_name.length()) + 4);
        } else if (inst.qubits.size() >= 2) {
            // 多量子比特门默认返回5
            return 5;
        }
    }
    
    return 5; // 默认宽度
}

// 辅助函数：构建列
static std::vector<Element> build_column_for_instruction(
    const CircuitInstruction& inst,
    int num_qubits,
    int num_clbits,
    int total_rows,
    int col_width = 3) {
    
    std::vector<Element> column(total_rows);
    
    // 初始化所有量子线
    for (int i = 0; i < num_qubits; ++i) {
        int row = i * 2;
        column[row] = QuantumWire(col_width);
        if (i < num_qubits - 1) {
            column[row + 1] = EmptySpace(col_width);
        }
    }
    
    // 总是初始化经典线
    int clbit_row_start = num_qubits * 2 - 1;
    column[clbit_row_start] = EmptySpace(col_width);
    column[clbit_row_start + 1] = ClassicalWire(col_width);
    column[clbit_row_start + 2] = EmptySpace(col_width);
    
    // 根据指令类型修改对应的行
    if (inst.is_gate()) {
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        std::string gate_name = gate->name();
        
        // 计算参数化门的标签
        std::string gate_label = gate_name;
        if (gate->has_parameters() && gate->parameter_count() > 0) {
            const Parameter& param = gate->get_parameter(0);
            std::stringstream ss;
            ss << gate_name << "(";
            
            auto var_names = param.get_variables();
            if (!var_names.empty()) {
                ss << var_names[0] << ":";
                ss << std::fixed << std::setprecision(2) << param.value();
            } else {
                ss << std::fixed << std::setprecision(2) << param.value();
            }
            
            ss << ")";
            gate_label = ss.str();
        }
        
        if (inst.qubits.size() == 1) {
            // 单量子比特门
            int qubit = inst.qubits[0];
            column[qubit * 2] = GateElement(gate_name);
            
        } else if (inst.qubits.size() == 2) {
            // 双量子比特门
            int q0 = inst.qubits[0];
            int q1 = inst.qubits[1];
            int min_q = std::min(q0, q1);
            int max_q = std::max(q0, q1);
            
            if (gate_name == "cx" || gate_name == "cnot") {
                column[q0 * 2] = ControlElement();
                column[q1 * 2] = TargetElement("X");
                
                // 连接线
                for (int i = min_q; i < max_q; ++i) {
                    if (i > min_q && i < max_q) {
                        column[i * 2] = CrossLine();
                    }
                    if (i < max_q && i * 2 + 1 < column.size()) {
                        column[i * 2 + 1] = VerticalLine();
                    }
                }
            } else if (gate_name == "swap") {
                column[q0 * 2] = SwapElement();
                column[q1 * 2] = SwapElement();
                
                // 连接线
                for (int i = min_q; i < max_q; ++i) {
                    if (i > min_q && i < max_q) {
                        column[i * 2] = CrossLine();
                    }
                    if (i < max_q && i * 2 + 1 < column.size()) {
                        column[i * 2 + 1] = VerticalLine();
                    }
                }
            } else {
                // 其他双量子比特门
                bool has_label = (gate_label != gate_name);
                
                if (has_label) {
                    // 带参数标签的门需要额外宽度
                    std::string elem = "──●";
                    for (int j = 3; j < col_width; ++j) elem += "─";
                    column[q0 * 2] = text(elem);
                    column[q1 * 2] = text(elem);
                    
                    // 连接线
                    for (int i = min_q; i < max_q; ++i) {
                        if (i > min_q && i < max_q) {
                            std::string cross_elem = "──┼";
                            for (int j = 3; j < col_width; ++j) cross_elem += "─";
                            column[i * 2] = text(cross_elem);
                        }
                        if (i < max_q && i * 2 + 1 < column.size()) {
                            column[i * 2 + 1] = VerticalLine();
                        }
                    }
                    
                    // 参数标签
                    if (max_q - min_q > 0) {
                        int label_row = min_q;
                        if (max_q - min_q > 1) {
                            label_row = (min_q + max_q) / 2;
                        }
                        
                        if (label_row * 2 + 1 < column.size()) {
                            std::string label_line = "  │" + gate_label;
                            while (label_line.length() < col_width) {
                                label_line += " ";
                            }
                            column[label_row * 2 + 1] = text(label_line);
                        }
                    }
                } else {
                    column[q0 * 2] = ControlElement();
                    column[q1 * 2] = ControlElement();
                    
                    // 连接线
                    for (int i = min_q; i < max_q; ++i) {
                        if (i > min_q && i < max_q) {
                            column[i * 2] = CrossLine();
                        }
                        if (i < max_q && i * 2 + 1 < column.size()) {
                            column[i * 2 + 1] = VerticalLine();
                        }
                    }
                }
            }
        } else if (inst.qubits.size() == 3) {
            // 三量子比特门
            int q0 = inst.qubits[0];
            int q1 = inst.qubits[1];
            int q2 = inst.qubits[2];
            int min_q = std::min({q0, q1, q2});
            int max_q = std::max({q0, q1, q2});
            
            if (gate_name == "ccx" || gate_name == "toffoli") {
                // CCX/Toffoli 门：两个控制点，一个目标点
                column[q0 * 2] = ControlElement();
                column[q1 * 2] = ControlElement();
                column[q2 * 2] = TargetElement("X");
                
                // 连接线
                for (int i = min_q; i < max_q; ++i) {
                    // 中间经过的量子比特（不是门的一部分）
                    bool is_gate_qubit = (i == q0 || i == q1 || i == q2);
                    if (i > min_q && i < max_q && !is_gate_qubit) {
                        column[i * 2] = CrossLine();
                    }
                    if (i < max_q && i * 2 + 1 < column.size()) {
                        column[i * 2 + 1] = VerticalLine();
                    }
                }
            } else {
                // 其他三量子比特门（通用处理）
                bool has_label = (gate_label != gate_name);
                
                // 三个连接点
                column[q0 * 2] = ControlElement();
                column[q1 * 2] = ControlElement();
                column[q2 * 2] = ControlElement();
                
                // 连接线
                for (int i = min_q; i < max_q; ++i) {
                    bool is_gate_qubit = (i == q0 || i == q1 || i == q2);
                    if (i > min_q && i < max_q && !is_gate_qubit) {
                        column[i * 2] = CrossLine();
                    }
                    if (i < max_q && i * 2 + 1 < column.size()) {
                        column[i * 2 + 1] = VerticalLine();
                    }
                }
                
                // 门标签（如果有参数）
                if (has_label && max_q - min_q > 0) {
                    int label_row = (min_q + max_q) / 2;
                    if (label_row * 2 + 1 < column.size()) {
                        std::string label_line = "  │" + gate_label;
                        while (label_line.length() < col_width) {
                            label_line += " ";
                        }
                        column[label_row * 2 + 1] = text(label_line);
                    }
                }
            }
        }
    } else if (inst.is_measurement()) {
        const auto& meas = std::get<Measurement>(inst.operation);
        // 注意：这里只处理第一个测量，如果有批量测量需要额外处理
        if (meas.size() > 0) {
            int qubit = meas.qubit_indices[0];
            int clbit = meas.clbit_indices[0];
            
            column[qubit * 2] = MeasurementElement();
            
            // 从测量比特到经典线的双垂直线
            for (int i = qubit; i < num_qubits; ++i) {
                if (i > qubit) {
                    column[i * 2] = text("──║──");
                }
                if (i < num_qubits - 1) {
                    column[i * 2 + 1] = DoubleVerticalLine();
                }
            }
            
            // 经典线连接
            column[clbit_row_start] = DoubleVerticalLine();
            column[clbit_row_start + 1] = ClassicalCross();
            column[clbit_row_start + 2] = text("  " + std::to_string(clbit) + "  ");
        }
    } else if (inst.is_barrier()) {
        const auto& barrier = std::get<Barrier>(inst.operation);
        std::vector<int> qubits = barrier.qubits;
        
        if (qubits.empty()) {
            // 全局Barrier
            for (int i = 0; i < num_qubits; ++i) {
                column[i * 2] = BarrierElement();
            }
        } else {
            // 部分Barrier
            for (int qubit : qubits) {
                column[qubit * 2] = BarrierElement();
            }
        }
    }
    
    return column;
}

std::string FTXUICircuitDrawer::draw() {
    std::vector<Elements> grid;
    
    int total_rows = num_qubits_ * 2 - 1 + 3; // 包含空行 + 经典寄存器线 + 比特索引行
    
    for (int i = 0; i < total_rows; ++i) {
        grid.push_back(Elements());
    }
    
    // 检查是否有测量指令
    bool has_measurements = false;
    for (const auto& inst : instructions_) {
        if (inst.is_measurement()) {
            has_measurements = true;
            break;
        }
    }
    
    // 第一列：量子比特 and 空行 and 经典比特
    for (int i = 0; i < num_qubits_; ++i) {
        int row = i * 2;
        grid[row].push_back(text("q[" + std::to_string(i) + "]: "));
        
        if (i < num_qubits_ - 1) {
            grid[row + 1].push_back(text("      "));
        }
    }

    int clbit_row_start = num_qubits_ * 2 - 1;
    grid[clbit_row_start].push_back(text("      "));  // 空行
    grid[clbit_row_start + 1].push_back(text("c: " + std::to_string(num_clbits_) + "/"));
    grid[clbit_row_start + 2].push_back(text("      "));  // 比特索引行
    
    // 第二列：初始线 and 空行 and 经典线
    int length = 3;
    for (int i = 0; i < num_qubits_; ++i) {
        int row = i * 2;
        grid[row].push_back(QuantumWire(length));
        
        // 空行
        if (i < num_qubits_ - 1) {
            grid[row + 1].push_back(EmptySpace(length));
        }
    }
    
    grid[clbit_row_start].push_back(EmptySpace(length));  // 空行
    grid[clbit_row_start + 1].push_back(ClassicalWire(length));
    grid[clbit_row_start + 2].push_back(EmptySpace(length));  // 比特索引行
    

    // circuit instructions
    for (const auto& inst : instructions_) {
        int col_width = calculate_column_width(inst);
        std::vector<Element> column = build_column_for_instruction(inst, num_qubits_, num_clbits_, total_rows, col_width);
        
        for (int row = 0; row < total_rows; ++row) {
            grid[row].push_back(column[row]);
        }
        
        // Measurement
        if (inst.is_measurement()) {
            const auto& meas = std::get<Measurement>(inst.operation);
            for (size_t idx = 1; idx < meas.size(); ++idx) {
                // 创建单个测量的指令
                Measurement single_measurement(meas.qubit_indices[idx], meas.clbit_indices[idx]);
                CircuitInstruction single_meas(single_measurement);
                
                // 构建并添加这个测量的列
                std::vector<Element> meas_column = build_column_for_instruction(single_meas, num_qubits_, num_clbits_, total_rows, 5);
                for (int row = 0; row < total_rows; ++row) {
                    grid[row].push_back(meas_column[row]);
                }
            }
        }
    }
    
    // 决定要显示多少行
    int rows_to_display = num_qubits_ * 2 - 1;
    if (num_clbits_ > 0) {
        rows_to_display += 2;
        if (has_measurements) {
            rows_to_display += 1;
        }
    }
    
    // 只取需要显示的行
    std::vector<Elements> display_grid;
    for (int i = 0; i < rows_to_display; ++i) {
        display_grid.push_back(grid[i]);
    }
    
    auto document = gridbox(display_grid);
    auto screen = Screen::Create(Dimension::Fit(document));
    Render(screen, document);
    
    return screen.ToString();
}

} // namespace qsteedcpp