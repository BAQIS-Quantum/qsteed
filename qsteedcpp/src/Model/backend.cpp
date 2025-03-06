#include "backend.h"
#include <iostream>



Backend::Backend() : c_list({std::make_tuple(0, 0, 0)}) {
   // std::cout << "Empty Backend Created" << std::endl;
}

Backend::Backend(CouplingList c_list) : c_list(c_list) {
   // std::cout << "Backend Created" << std::endl;
}

Backend::Backend(CouplingList c_list, int qubits_num) :
    c_list(c_list), qubits_num(qubits_num) {}