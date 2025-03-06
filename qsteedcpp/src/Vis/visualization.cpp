#ifdef WITH_GRAPHVIZ

#include "visualization.h"
#include "writer.h"
#include <iostream>
#include <sstream>
#include <vector>

#include <graphviz/gvc.h>

namespace Vis {

    void draw_graph(const DagGraph& graph) { 
        draw_dot(graph_to_dot(graph));
    }
    void draw_graph(const CouplingGraph& graph) { 
        draw_dot(graph_to_dot(graph));
    }

    std::string vectorToString(const std::vector<int>& vec) {
        std::ostringstream ss;
        for (size_t i = 0; i < vec.size(); ++i) 
        {
            if (i > 0)  ss << ",";
            ss << vec[i]; 
        }
        return ss.str();
    }

    std::string graph_to_dot(const DagGraph& graph) {
        std::ostringstream os;
        boost::write_graphviz(os, graph, DagNodeWriter(graph), DagEdgeWriter(graph));
        return os.str();
    }

    std::string graph_to_dot(const CouplingGraph& graph) {
        std::ostringstream os;
        boost::write_graphviz(os, graph, CouplingNodeWriter(graph), CouplingEdgeWriter(graph));
        return os.str();
    }

    void draw_dot(const std::string dot_str) {
        std::cout << "-- drawing --" << std::endl;

        // save .dot file
        std::ofstream dot_file("output.dot");
        dot_file << dot_str;
        dot_file.close();

        GVC_t *gvc = gvContext();
        // 使用 agmemread() 读取 Graphviz 字符串, c_str() in Temporary buffer
        Agraph_t *g = agmemread(dot_str.c_str());

        // 设置布局引擎
        gvLayout(gvc, g, "dot");

        // Render img & save as .pnd
        gvRenderFilename(gvc, g, "png", "output.png");

        // Free layout resource
        gvFreeLayout(gvc, g);
        agclose(g);
        gvFreeContext(gvc);
    }

}

#endif