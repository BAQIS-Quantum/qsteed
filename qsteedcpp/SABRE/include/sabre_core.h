#pragma once
#include <utility>
#include <cstdint>
#include <unordered_map>
#include <string>

namespace sabre {

    using qubit_t = int;
    using cbit_t = int;
    using node_pos_t = uint64_t;
    using edge_pos_t = std::pair<node_pos_t, node_pos_t>;
    using SwapPos = std::pair<int, int>;

    struct SwapPosHash {
        template <class T1, class T2>
        std::size_t operator() (const std::pair<T1, T2> &p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ (h2 << 1);
        }
    };

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