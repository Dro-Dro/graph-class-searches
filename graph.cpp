#include "graph.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

// constructor, empty graph
// directionalEdges defaults to true
Graph::Graph(bool directionalEdges) { isDirectional = directionalEdges; }

// destructor
Graph::~Graph() {
  for (auto &s : vertices) {
    delete s.second;
  }
  for (auto &j : edges) {
    delete j;
  }
}

// @return total number of vertices
int Graph::verticesSize() const { return vertices.size(); }

// @return total number of edges
int Graph::edgesSize() const { return edges.size(); }

// @return number of edges from given vertex, -1 if vertex not found
int Graph::vertexDegree(const string &label) const {
  if (contains(label)) {
    return vertices.at(label)->connectedVerts.size();
  } else {
    return -1;
  }
}

// @return true if vertex added, false if it already is in the graph
bool Graph::add(const string &label) {
  if (contains(label)) {
    return false;
  } else {
    Vertex *new_vertex = new Vertex();
    new_vertex->label = label;
    vertices[label] = new_vertex;
    return true;
  }
}

/** return true if vertex already in graph */
bool Graph::contains(const string &label) const {
  if (vertices.count(label) == 1) {
    return true;
  } else {
    return false;
  }
}

// return true if edge exists
bool Graph::containsEdge(const string &from, const string &to,
                         int weight = 0) const {
  Edge checkEdge;
  checkEdge.from = from;
  checkEdge.to = to;
  checkEdge.weight = weight;
  for (auto &s : edges) {
    if (s->from == checkEdge.from && s->to == checkEdge.to) {
      return true;
    }
  }
  return false;
}

// @return string representing edges and weights, "" if vertex not found
// A-3->B, A-5->C should return B(3),C(5)
string Graph::getEdgesAsString(const string &label) const {
  if (!contains(label)) {
    return "";
  } else {
    map<string, string> z;
    string p;
    for (auto &s : vertices.at(label)->connectedVerts) {
      string o;
      o += s.first->label + "(";
      o += to_string(s.second);
      o += "),";
      z[s.first->label] = o;
    }
    for (auto &i : z) {
      p += i.second;
    }
    p = p.substr(0, p.size() - 1);
    return p;
  }
}

// @return true if successfully connected
bool Graph::connect(const string &from, const string &to, int weight) {
  if (containsEdge(from, to, weight) || from == to) {
    return false;
  } else {
    if (!contains(from)) {
      add(from);
    }
    if (!contains(to)) {
      add(to);
    }
    Edge *new_edge = new Edge();
    new_edge->from = from;
    new_edge->to = to;
    new_edge->weight = weight;
    edges.push_back(new_edge);
    pair<Vertex *, int> conVert;
    conVert.first = vertices[to];
    conVert.second = weight;
    vertices[from]->connectedVerts.push_back(conVert);
    if (!isDirectional) {
      connect(to, from, weight);
    }
    return true;
  }
}

bool Graph::disconnect(const string &from, const string &to) {
  if (!containsEdge(from, to) || from == to) {
    return false;
  } else {
    for (auto &s : edges) {
      if (s->from == from && s->to == to) {
        delete s;
        edges.erase(find(edges.begin(), edges.end(), s));
      }
    }
    for (auto &s : vertices.at(from)->connectedVerts) {
      if (s.first->label == to) {
        s.first = nullptr;
        vertices[from]->connectedVerts.erase(
            find(vertices[from]->connectedVerts.begin(),
                 vertices[from]->connectedVerts.end(), s));
      }
    }
    if (!isDirectional) {
      disconnect(to, from);
    }
    return true;
  }
}

// depth-first traversal starting from given startLabel
void Graph::dfs(const string &startLabel, void visit(const string &label)) {
  visited[startLabel] = true;
  for (auto &s : vertices.at(startLabel)->connectedVerts) {
    if (visited.count(s.first->label) == 0) {
      dfs(s.first->label, visit);
    }
  }
}

// breadth-first traversal starting from startLabel
void Graph::bfs(const string &startLabel, void visit(const string &label)) {
  queue<string> q;
  q.push(startLabel);
  visited[startLabel] = true;
  while (!q.empty()) {
    string u = q.front();
    q.pop();
    for (auto &s : vertices.at(startLabel)->connectedVerts) {
      if (visited.count(s.first->label) == 0) {
        visited[s.first->label] = true;
        q.push(s.first->label);
      }
    }
  }
}

// store the weights in a map
// store the previous label in a map
pair<map<string, int>, map<string, string>>
Graph::dijkstra(const string &startLabel) const {
  map<string, int> weights;
  map<string, string> previous;
  // TODO(student) Your code here
  set<pair<string, int>> extract;
  extract.insert(make_pair(startLabel, 0));
  map<string, int> weightsInPath;
  weightsInPath[startLabel] = 0;
  int currentPath = INT32_MAX;
  while (!extract.empty()) {
    pair<string, int> tmp = *(extract.begin());
    extract.erase(extract.begin());
    string u = tmp.first;
    for (auto &s : vertices.at(u)->connectedVerts) {
      string v = s.first->label;
      int weight = s.second;
      if (currentPath > weightsInPath[u] + weight) {
        if (currentPath != INT32_MAX) {
          extract.erase(extract.find(make_pair(v, weight)));
        }
        currentPath = weightsInPath[u] + weight;
        extract.insert(make_pair(v, weight));
        weights[v] = weight;
        previous[v] = u;
      }
    }
  }
  return make_pair(weights, previous);
}

// minimum spanning tree using Prim's algorithm
int Graph::mstPrim(const string &startLabel,
                   void visit(const string &from, const string &to,
                              int weight)) const {
  priority_queue<pair<int, string>> q;
  map<string, bool> visitedP;
  q.push(make_pair(0, startLabel));
  int mst_cost = 0;
  while (!q.empty()) {
    pair<int, string> tmp;
    tmp = q.top();
    q.pop();
    string vert = tmp.second;
    int cost = tmp.first;
    if (visitedP.count(vert) != 1) {
      mst_cost += cost;
      visitedP[vert] = true;
      for (auto &s : vertices.at(vert)->connectedVerts) {
        string adj_str = s.first->label;
        int adj_int = s.second;
        if (visited.count(adj_str) != 1) {
          q.push(make_pair(adj_int, adj_str));
        }
      }
    }
  }
  return -1;
}

// minimum spanning tree using Prim's algorithm
int Graph::mstKruskal(const string &startLabel,
                      void visit(const string &from, const string &to,
                                 int weight)) const {
  return -1;
}

// read a text file and create the graph
bool Graph::readFile(const string &filename) {
  ifstream myfile(filename);
  if (!myfile.is_open()) {
    cerr << "Failed to open " << filename << endl;
    return false;
  }
  int edges = 0;
  int weight = 0;
  string fromVertex;
  string toVertex;
  myfile >> edges;
  for (int i = 0; i < edges; ++i) {
    myfile >> fromVertex >> toVertex >> weight;
    connect(fromVertex, toVertex, weight);
  }
  myfile.close();
  return true;
}
