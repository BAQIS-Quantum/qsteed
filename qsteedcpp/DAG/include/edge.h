#pragma once
#include "instructionNode.h"

struct EdgeProperties 
{
public:
    qubit_t qubit_id; // 1,2,3 for qubits 

public:
    EdgeProperties() : qubit_id(-1) {}
    EdgeProperties(int qubit_id) : qubit_id(qubit_id) {}

};


struct Edge 
/*
    A struct to represent an edge in the DAG graph. 
    Just for convenience, we use a struct to store the edge information.
    It contains the source node, target node and the edge properties.
*/
{
public:
    node_pos_t source;    
    node_pos_t target;
    EdgeProperties ep;

public:
    Edge(node_pos_t source, node_pos_t target, EdgeProperties ep) 
        : source(source), target(target), ep(ep){
    }
    Edge(node_pos_t source, node_pos_t target, int qubit_id) 
        : source(source), target(target), ep(EdgeProperties(qubit_id)){
    }
};