#pragma once
#include <string>
#include "coupling.h"



class Backend
{
private:
    CouplingList c_list;
    int qubits_num;
    std::string backend_type;

public:
    Backend();
    Backend(CouplingList c_list); 
    Backend(CouplingList c_list, int qubits_num); 
    std::string name;
};