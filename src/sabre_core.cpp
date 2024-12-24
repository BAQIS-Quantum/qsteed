#include "sabre_core.h"

namespace sabre {

    Heuristic string2Heuristic(const std::string& heuristic) {
        if (heuristic == "fidelity") {
            return Heuristic::FIDELITY;
        } else if (heuristic == "distance") {
            return Heuristic::DISTANCE;
        } else if (heuristic == "mixture") {
            return Heuristic::MIXTURE;
        } else {
            throw std::invalid_argument("Invalid heuristic value: " + heuristic);
        }
    }


    InitialLayoutMethod string2InitialLayoutMethod(const std::string& method) {
        if (method == "random") {
            return InitialLayoutMethod::RANDOM;
        } else if (method == "fidelity") {
            return InitialLayoutMethod::FIDELITY;
        } else if (method == "dense") {
            return InitialLayoutMethod::DENSE;
        } else {
            throw std::invalid_argument("Invalid initial layout method: " + method);
        }
    }

}