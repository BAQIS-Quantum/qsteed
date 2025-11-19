#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <iostream>


using LayoutStructure = std::unordered_map<int, int>;


// Layout class 
// 默认构造函数 和 []运算符重载 只为v2p服务
class Layout
{
private:
    LayoutStructure v2p{};
    LayoutStructure p2v{};

public:
    Layout() = default;
    Layout(LayoutStructure layout_struc) : v2p(layout_struc) {
        for (const auto& pair : v2p)
            p2v[pair.second] = pair.first;
    } 
    
    const int& operator[](int v) const { 
        return v2p.at(v); 
    }

    bool empty() const { 
        return v2p.empty() && p2v.empty(); 
    }

    const LayoutStructure& get_v2p() const { 
        return v2p; 
    }

    const LayoutStructure& get_p2v() const {
        return p2v;
    }

    // Get physical position assuming a hypothetical swap (without modifying layout)
    int get_with_swap(int virt, int swap_a, int swap_b) const {
        if (virt == swap_a) return v2p.at(swap_b);
        if (virt == swap_b) return v2p.at(swap_a);
        return v2p.at(virt);
    }

    void set_v2p( const LayoutStructure& v2p) { 
        this->v2p = v2p; 
        _update_p2v();
    }

    void set_p2v( const LayoutStructure& p2v) { 
        this->p2v = p2v; 
        _updata_v2p();
    }

    void swap(int a, int b) {
        int phys_a = v2p[a];
        int phys_b = v2p[b];

        std::swap(v2p[a], v2p[b]);

        // Update only the two affected entries - O(1) instead of O(N)
        p2v[phys_a] = b;
        p2v[phys_b] = a;
    }


private:
    void _update_p2v() {
        p2v.clear();
        for (const auto& pair : v2p)
            p2v[pair.second] = pair.first;
    }

    void _updata_v2p() {
        v2p.clear();
        for (const auto& pair : p2v)
            v2p[pair.second] = pair.first;
    }
};



//* Utility functions
Layout generate_random_layout(int virtual_qubits, int physical_qubits);

void print_layout(const LayoutStructure& layout); 