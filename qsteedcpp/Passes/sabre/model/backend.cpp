#include "backend.h"

Backend::Backend() : c_list({std::make_tuple(0, 0, 0)}) {}

Backend::Backend(CouplingList c_list) : c_list(c_list) {}

Backend::Backend(CouplingList c_list, int qubits_num) :
    c_list(c_list), qubits_num(qubits_num) {}