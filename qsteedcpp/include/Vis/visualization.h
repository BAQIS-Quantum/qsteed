#pragma once

#ifdef WITH_GRAPHVIZ
#include <string>
#include <vector>
#include "sabre_core.h"
#include "DAG/dag.h"
#include "coupling.h"


namespace Vis {

    /**
     * @brief Draws a graph using the specified graph object.
     * @tparam Graph The type of graph object. It can be either DagGraph or CouplingGraph.
     * @param graph The graph object to be drawn.
     */
    void draw_graph(const DagGraph& graph);
    void draw_graph(const CouplingGraph& graph);

    
    /**
     * @brief Converts a vector of integers to a string representation.
     * @param vec The vector of integers.
     * @return The string representation of the vector.
     */
    std::string vectorToString(const std::vector<int>& vec);


    /**
     * @brief Converts a DAG/Coupling graph to a DOT string representation.
     * @param graph The DAG/Coupling graph.
     * @return The DOT string representation of the graph.
     */
    std::string graph_to_dot(const DagGraph& graph);
    std::string graph_to_dot(const CouplingGraph& graph);


    /**
     * @brief Draws a graph using the specified DOT string.
     * @param dot_str The DOT string representation of the graph.
     */
    void draw_dot(const std::string dot_str);
}

#endif