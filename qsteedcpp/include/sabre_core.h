#pragma once
#include <utility>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <stdexcept>

namespace sabre {

    using qubit_t = int;
    using cbit_t = int;
    using node_pos_t = uint64_t;
    using edge_pos_t = std::pair<node_pos_t, node_pos_t>;
    using SwapPos = std::pair<int, int>;

    using RawLayout = std::unordered_map<int, int>;

    enum class Heuristic {
        FIDELITY,
        DISTANCE,
        MIXTURE,
    };

    enum class InitialLayoutMethod {
        RANDOM,
        FIDELITY,
        DENSE,
    };


    Heuristic string2Heuristic(const std::string& heristic);

    InitialLayoutMethod string2InitialLayoutMethod(const std::string& method);


};