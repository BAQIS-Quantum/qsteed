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

    void set_v2p( const LayoutStructure& v2p) { 
        this->v2p = v2p; 
        _update_p2v();
    }

    void set_p2v( const LayoutStructure& p2v) { 
        this->p2v = p2v; 
        _updata_v2p();
    }

    void swap(int a, int b) {
        std::swap(v2p[a], v2p[b]);
        _update_p2v();      
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