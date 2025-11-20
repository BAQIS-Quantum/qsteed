#pragma once
#include <torch/torch.h>
#include <cmath>
#include <vector>
#include "gates/base_gate.h"

namespace qsteedcpp {

// Differentiable version: accepts torch::Tensor parameters
// This preserves autograd computation graph
inline torch::Tensor make_gate_matrix_differentiable(
    GateType gate_type,
    const std::vector<torch::Tensor>& params = {}) {

    using c = c10::complex<double>;

    auto make_2x2 = [](c10::complex<double> a, c10::complex<double> b,
                       c10::complex<double> c, c10::complex<double> d) {
        auto m = torch::zeros({2, 2}, torch::kComplexDouble);
        m[0][0] = a;
        m[0][1] = b;
        m[1][0] = c;
        m[1][1] = d;
        return m;
    };

    switch (gate_type) {
        // Single-qubit non-parametric gates (same as before)
        case GateType::H: {
            double s = 1.0 / std::sqrt(2.0);
            return make_2x2({s, 0}, {s, 0}, {s, 0}, {-s, 0});
        }

        case GateType::X:
            return make_2x2({0, 0}, {1, 0}, {1, 0}, {0, 0});

        case GateType::Y:
            return make_2x2({0, 0}, {0, -1}, {0, 1}, {0, 0});

        case GateType::Z:
            return make_2x2({1, 0}, {0, 0}, {0, 0}, {-1, 0});

        case GateType::S:
            return make_2x2({1, 0}, {0, 0}, {0, 0}, {0, 1});

        case GateType::SDG:
            return make_2x2({1, 0}, {0, 0}, {0, 0}, {0, -1});

        case GateType::T: {
            double cos_val = std::cos(M_PI / 4);
            double sin_val = std::sin(M_PI / 4);
            return make_2x2({1, 0}, {0, 0}, {0, 0}, {cos_val, sin_val});
        }

        case GateType::TDG: {
            double cos_val = std::cos(-M_PI / 4);
            double sin_val = std::sin(-M_PI / 4);
            return make_2x2({1, 0}, {0, 0}, {0, 0}, {cos_val, sin_val});
        }

        // Parametric gates: USE TORCH OPS for differentiability
        case GateType::RX: {
            torch::Tensor theta = params[0];
            torch::Tensor half = theta / 2.0;
            torch::Tensor cos_val = torch::cos(half);  // ✅ torch::cos preserves grad
            torch::Tensor sin_val = torch::sin(half);  // ✅ torch::sin preserves grad

            // Build matrix using torch::stack to preserve gradients
            // 确保所有complex tensor都是kComplexDouble
            auto minus_i = torch::tensor(c(0, -1), torch::kComplexDouble);
            auto plus_i = torch::tensor(c(0, 1), torch::kComplexDouble);

            auto zero = torch::zeros_like(cos_val);
            auto elem00 = torch::complex(cos_val, zero).to(torch::kComplexDouble);
            auto elem01 = minus_i * torch::complex(sin_val, zero).to(torch::kComplexDouble);
            auto elem10 = plus_i * torch::complex(sin_val, zero).to(torch::kComplexDouble);
            auto elem11 = torch::complex(cos_val, zero).to(torch::kComplexDouble);

            auto row0 = torch::stack({elem00, elem01}, 0);
            auto row1 = torch::stack({elem10, elem11}, 0);
            return torch::stack({row0, row1}, 0);
        }

        case GateType::RY: {
            torch::Tensor theta = params[0];
            torch::Tensor half = theta / 2.0;
            torch::Tensor cos_val = torch::cos(half);
            torch::Tensor sin_val = torch::sin(half);

            // Build matrix using torch::stack to preserve gradients
            auto zero = torch::zeros_like(cos_val);
            auto elem00 = torch::complex(cos_val, zero).to(torch::kComplexDouble);
            auto elem01 = torch::complex(-sin_val, zero).to(torch::kComplexDouble);
            auto elem10 = torch::complex(sin_val, zero).to(torch::kComplexDouble);
            auto elem11 = torch::complex(cos_val, zero).to(torch::kComplexDouble);

            auto row0 = torch::stack({elem00, elem01}, 0);
            auto row1 = torch::stack({elem10, elem11}, 0);
            return torch::stack({row0, row1}, 0);
        }

        case GateType::RZ: {
            torch::Tensor lambda = params[0];
            torch::Tensor half = lambda / 2.0;

            // e^(-iλ/2) = cos(-λ/2) + i*sin(-λ/2)
            torch::Tensor cos_minus = torch::cos(-half);
            torch::Tensor sin_minus = torch::sin(-half);
            // e^(iλ/2) = cos(λ/2) + i*sin(λ/2)
            torch::Tensor cos_plus = torch::cos(half);
            torch::Tensor sin_plus = torch::sin(half);

            // Build matrix using torch::stack to preserve gradients
            auto zero = torch::zeros_like(cos_minus);
            auto elem00 = torch::complex(cos_minus, sin_minus).to(torch::kComplexDouble);
            auto elem01 = torch::complex(zero, zero).to(torch::kComplexDouble);
            auto elem10 = torch::complex(zero, zero).to(torch::kComplexDouble);
            auto elem11 = torch::complex(cos_plus, sin_plus).to(torch::kComplexDouble);

            auto row0 = torch::stack({elem00, elem01}, 0);
            auto row1 = torch::stack({elem10, elem11}, 0);
            return torch::stack({row0, row1}, 0);
        }

        case GateType::P: {
            torch::Tensor lambda = params[0];
            torch::Tensor cos_val = torch::cos(lambda);
            torch::Tensor sin_val = torch::sin(lambda);

            // Build matrix using torch::stack to preserve gradients
            auto zero = torch::zeros_like(cos_val);
            auto one = torch::ones_like(cos_val);
            auto elem00 = torch::complex(one, zero).to(torch::kComplexDouble);
            auto elem01 = torch::complex(zero, zero).to(torch::kComplexDouble);
            auto elem10 = torch::complex(zero, zero).to(torch::kComplexDouble);
            auto elem11 = torch::complex(cos_val, sin_val).to(torch::kComplexDouble);

            auto row0 = torch::stack({elem00, elem01}, 0);
            auto row1 = torch::stack({elem10, elem11}, 0);
            return torch::stack({row0, row1}, 0);
        }

        // Two-qubit gates
        case GateType::CNOT:
            return torch::tensor({
                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 0, 1},
                {0, 0, 1, 0}
            }, torch::kComplexDouble);

        case GateType::CZ:
            return torch::tensor({
                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, -1}
            }, torch::kComplexDouble);

        case GateType::SWAP:
            return torch::tensor({
                {1, 0, 0, 0},
                {0, 0, 1, 0},
                {0, 1, 0, 0},
                {0, 0, 0, 1}
            }, torch::kComplexDouble);

        case GateType::ISWAP: {
            auto m = torch::zeros({4, 4}, torch::kComplexDouble);
            m[0][0] = c(1, 0);
            m[1][2] = c(0, 1);
            m[2][1] = c(0, 1);
            m[3][3] = c(1, 0);
            return m;
        }

        default:
            throw std::runtime_error("Unsupported gate type");
    }
}

}  // namespace qsteedcpp
