#include "graph.h"
#include "graphT.h"

#include <algorithm>

using namespace OSG;

template class graph<graph_base::emptyNode>;

graph_base::graph_base() {}
graph_base::~graph_base() {}

void graph_base::connect(int i, int j, CONNECTION c) {
    if (i >= nodes.size() || j >= nodes.size()) return;
    while (i >= edges.size()) edges.push_back( vector<edge>() );
    edges[i].push_back(edge(i,j,c));
}

void graph_base::disconnect(int i, int j) {
    if (i >= nodes.size() || j >= nodes.size()) return;
    if (i >= edges.size()) return;
    auto& v = edges[i];
    for (int k=0; k<v.size(); k++) {
        if (v[k].to == j) {
            v.erase(v.begin()+k);
            break;
        }
    }
}

vector< vector< graph_base::edge > >& graph_base::getEdges() { return edges; }
vector< graph_base::node >& graph_base::getNodes() { return nodes; }
graph_base::node& graph_base::getNode(int i) { return nodes[i]; }
void graph_base::update(int i, bool changed) {}
int graph_base::size() { return nodes.size(); }
void graph_base::clear() { nodes.clear(); edges.clear(); }

int graph_base::getNEdges() {
    int N = 0;
    for (auto& n : edges) N += n.size();
    return N;
}

void graph_base::setPosition(int i, Vec3f v) {
    auto& n = nodes[i];
    n.box.setCenter(v);
    update(i, true);
}

graph_base::edge::edge(int i, int j, CONNECTION c) : from(i), to(j), connection(c) {}

//vector<graph_base::node>::iterator graph_base::begin() { return nodes.begin(); }
//vector<graph_base::node>::iterator graph_base::end() { return nodes.end(); }
