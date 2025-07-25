#include "circuit/ftxui_interactive_drawer.h"
#include "circuit/ftxui_circuit_drawer.h"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include <sstream>
#include <vector>
#include <string>

using namespace ftxui;

namespace qsteedcpp {

Component FTXUIInteractiveDrawer::create_component() {
    // 创建可滚动的渲染器
    auto circuit_renderer = Renderer([this] {
        // 生成电路文本
        FTXUICircuitDrawer drawer(instructions_, num_qubits_, num_clbits_);
        std::string circuit_text = drawer.draw();
        
        // 将文本按行分割并转换为 Element
        std::vector<Element> lines;
        std::stringstream ss(circuit_text);
        std::string line;
        while (std::getline(ss, line)) {
            lines.push_back(text(line));
        }
        auto circuit_element = vbox(std::move(lines));
        
        auto content = vbox({
            hbox({
                text("Quantum Circuit Viewer") | bold,
                text(" (use ←→↑↓ to scroll, Page Up/Down for fast scroll, Home to reset, q to quit)") | dim
            }) | center,
            separator(),
            circuit_element | focusPositionRelative(scroll_x_, scroll_y_) | frame | flex,
            separator(),
            render_status_bar()
        });
        
        return content | border;
    });
    
    // 添加键盘控制
    circuit_renderer |= CatchEvent([this](Event event) {
        const float small_step = 0.05f;
        const float large_step = 0.2f;
        
        if (event == Event::ArrowLeft) {
            scroll_x_ = std::max(0.0f, scroll_x_ - small_step);
            return true;
        } else if (event == Event::ArrowRight) {
            scroll_x_ = std::min(1.0f, scroll_x_ + small_step);
            return true;
        } else if (event == Event::ArrowUp) {
            scroll_y_ = std::max(0.0f, scroll_y_ - small_step);
            return true;
        } else if (event == Event::ArrowDown) {
            scroll_y_ = std::min(1.0f, scroll_y_ + small_step);
            return true;
        } else if (event == Event::PageUp) {
            scroll_y_ = std::max(0.0f, scroll_y_ - large_step);
            return true;
        } else if (event == Event::PageDown) {
            scroll_y_ = std::min(1.0f, scroll_y_ + large_step);
            return true;
        } else if (event == Event::Home) {
            scroll_x_ = 0.0f;
            scroll_y_ = 0.0f;
            return true;
        } else if (event == Event::End) {
            scroll_x_ = 1.0f;
            scroll_y_ = 1.0f;
            return true;
        } else if (event == Event::Character('q') || event == Event::Escape) {
            return true;  // Will be handled by the main event handler
        }
        return false;
    });
    
    return circuit_renderer;
}

Element FTXUIInteractiveDrawer::render_status_bar() {
    // 计算电路统计信息
    int gate_count = 0;
    int measurement_count = 0;
    int barrier_count = 0;
    
    for (const auto& inst : instructions_) {
        if (inst.is_gate()) {
            gate_count++;
        } else if (inst.is_measurement()) {
            measurement_count++;
        } else if (inst.is_barrier()) {
            barrier_count++;
        }
    }
    
    // 创建状态栏
    return hbox({
        text("Qubits: ") | bold,
        text(std::to_string(num_qubits_)),
        text(" | "),
        text("Gates: ") | bold,
        text(std::to_string(gate_count)),
        text(" | "),
        text("Measurements: ") | bold,
        text(std::to_string(measurement_count)),
        text(" | "),
        text("Scroll: ") | bold,
        text("[" + std::to_string(static_cast<int>(scroll_x_ * 100)) + "%, " + 
             std::to_string(static_cast<int>(scroll_y_ * 100)) + "%]"),
    }) | color(Color::Cyan);
}

void FTXUIInteractiveDrawer::run() {
    auto screen = ScreenInteractive::Fullscreen();
    auto component = create_component();
    
    // 添加退出处理
    component |= CatchEvent([&screen](Event event) {
        if (event == Event::Character('q') || event == Event::Escape) {
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });
    
    screen.Loop(component);
}

} // namespace qsteedcpp