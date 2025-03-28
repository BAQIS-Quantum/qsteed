#pragma once
#include <dag.h>

    class DagNodeWriter {
    public:
        DagGraph g;

    public:
        DagNodeWriter(const DagGraph& g) : g(g) {}

        template <typename VertexDescriptor, typename OutputStream>
        void operator()(OutputStream& os, VertexDescriptor v) const {
            const auto& node = g[v];
            os << "[label=\"" << v;
            os << "{" << node.name; 
            os << "("<< vectorToString(node.qubit_pos);
            os << ")" << "}" << "\"]";
        }
    };

    class DagEdgeWriter {
    public:
        DagGraph g;
        DagEdgeWriter(DagGraph g) : g(g) {}

        template <typename EdgeDescriptor, typename OutputStream>
        void operator()(OutputStream& os, EdgeDescriptor e) const 
        {
            const auto& edge = g[e];
            os << "[label=\"" << "q" << edge.qubit_id << "\"]";
        }
    };


    class CouplingNodeWriter {
    public:
        CouplingGraph g;

    public:
        CouplingNodeWriter(const CouplingGraph& g) : g(g) {}

        template <typename VertexDescriptor, typename OutputStream>
        void operator()(OutputStream& os, VertexDescriptor v) const 
        {
            const auto& node = g[v];
            os << "[label=\"" << node.id << "\"]";
        }
    };

    class CouplingEdgeWriter {
    public:
        CouplingGraph g;

    public:
        CouplingEdgeWriter(const CouplingGraph& g) : g(g) {}

        template <typename EdgeDescriptor, typename OutputStream>
        void operator()(OutputStream& os, EdgeDescriptor e) const 
        {
            const auto& edge = g[e];
            os << "[label=\"" << edge.fidelity << "\"]";
        }
    };
