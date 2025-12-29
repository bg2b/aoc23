// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit1 doit1.cc
// ./doit1 1 < input  # part 1
// ./doit1 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <queue>
#include <algorithm>
#include <cstdint>
#include <cassert>

using namespace std;

using coord = array<int, 2>;

coord dirs[4] = {coord{+1, 0}, coord{0, +1}, coord{-1, 0}, coord{0, -1}};

coord operator+(coord const &c1, coord const &c2) {
  return {c1[0] + c2[0], c1[1] + c2[1]};
}

coord required_dir(char ch) {
  auto pos = string("v>^<").find(ch);
  assert(pos != string::npos);
  return dirs[pos];
}

// This is a smarter search than doit.cc, using a maximum spanning
// tree to get a best-case bound.  The search state consists of a tip
// node n, a visited set v (not containing n), and the number of steps
// so far.  For such a state, compute the maximum spanning tree that
// does not include anything in v (so it will include n).  The total
// length of the edges in the tree plus the steps so far provides a
// bound on the longest path.

// List of successor node index, number of steps to get there
using edges = vector<pair<int, int>>;
// Coordinate of the node, what it connects to
using node = pair<coord, edges>;
// A single edge: from node, to node, number of steps
using edge = tuple<int, int, int>;
// A bit set used to keep track of what nodes have been visited.
// There aren't many nodes, so a 64-bit int suffices if I just number
// then sequentially from 0.
using visited = uint64_t;

struct trail_map {
  // Can slopes be climbed?
  bool slippery;
  // The input map, directly as read
  vector<string> trails;
  // Start is always here
  coord start{0, 1};
  // Finish is at the right side of the last row
  coord finish;
  // Representation after compiling to graph form for speed
  vector<node> nodes;
  // All undirected graph edges sorted max steps to min steps, for
  // maximum spanning tree computation
  vector<edge> all_edges;

  trail_map(bool slippery_);

  int height() const { return trails.size(); }
  int width() const { return trails.front().length(); }
  bool in_bounds(coord const &c) const;
  char at(coord const &c) const { return trails[c[0]][c[1]]; }

  // Find the index for the node with coordinate c, return -1 if none
  int index(coord const &c) const;
  // Create nodes and link everything together
  void build_graph();
  // Write the graph in dot format for visualization
  void dot() const;

  // Return the total of all the edge lengths in the maximum spanning
  // tree after removing the nodes in v
  int max_spanning_tree(visited v) const;

  // Return the longest path from state to finish
  int longest_path() const;
};

trail_map::trail_map(bool slippery_) : slippery(slippery_) {
  string line;
  while (getline(cin, line)) {
    trails.push_back(line);
    assert(line.length() == trails.front().length());
  }
  assert(height() >= 2 && width() >= 2);
  finish = coord{height() - 1, width() - 2};
  assert(at(start) == '.');
  assert(at(finish) == '.');
  build_graph();
}

bool trail_map::in_bounds(coord const &c) const {
  return c[0] >= 0 && c[0] < height() && c[1] >= 0 && c[1] < width();
}

int trail_map::index(coord const &c) const {
  auto p = find_if(nodes.begin(), nodes.end(),
                   [&](node const &n) { return n.first == c; });
  if (p == nodes.end())
    return -1;
  return p - nodes.begin();
}

void trail_map::build_graph() {
  // Start and finish get fixed indices
  nodes.emplace_back(start, edges());
  nodes.emplace_back(finish, edges());
  assert(index(start) == 0);
  assert(index(finish) == 1);
  // Look for places where there's a possible branch; those are the
  // nodes
  for (int i = 0; i < height(); ++i)
    for (int j = 0; j < width(); ++j) {
      coord c{i, j};
      if (at(c) == '#' || c == start || c == finish)
        continue;
      int num_adjacent = 0;
      for (auto const dir : dirs)
        if (at(c + dir) != '#')
          ++num_adjacent;
      assert(num_adjacent != 1);
      if (num_adjacent > 2)
        nodes.emplace_back(c, edges());
    }
  assert(nodes.size() < 8 * sizeof(uint64_t));
  // Link nodes together, recording the number of steps needed
  for (auto &[c, adj] : nodes) {
    if (c == finish)
      // Finish doesn't need outgoing edges
      continue;
    for (auto const &dir : dirs) {
      auto nc = c + dir;
      if (!in_bounds(nc) || at(nc) == '#')
        // Invalid step
        continue;
      if (slippery && at(nc) != '.' && nc + required_dir(at(nc)) == c)
        // Can't go up a slippery slope
        continue;
      // Walk from c to some other node; there's only one valid choice
      // at each step
      auto last = c;
      int steps = 1;
      while (index(nc) == -1) {
        for (auto const &ndir : dirs)
          if (nc + ndir != last && at(nc + ndir) != '#') {
            last = nc;
            nc = nc + ndir;
            ++steps;
            break;
          }
      }
      adj.emplace_back(index(nc), steps);
    }
  }
  // Collect all edges
  for (auto const &[c, adj] : nodes) {
    int i = index(c);
    for (auto [j, steps] : adj) {
      int ii = i;
      int jj = j;
      if (ii > jj)
        swap(ii, jj);
      edge e{ii, jj, steps};
      if (find(all_edges.begin(), all_edges.end(), e) == all_edges.end())
        all_edges.push_back(e);
    }
  }
  // Sort for maximum spanning tree computation
  sort(all_edges.begin(), all_edges.end(),
       [](edge const &e1, edge const &e2) {
         auto [i1, j1, steps1] = e1;
         auto [i2, j2, steps2] = e2;
         if (steps1 != steps2)
           return steps1 > steps2;
         if (i1 != i2)
           return i1 < i2;
         return j1 < j2;
       });
}


void trail_map::dot() const {
  cout << (slippery ? "di" : "") << "graph G {\n";
  auto name = [](coord const &c) {
                return '"' + to_string(c[0]) + ',' + to_string(c[1]) + '"';
              };
  auto edge = slippery ? " -> " : " -- ";
  for (auto const &[c, adj] : nodes)
    for (auto [next_index, steps] : adj) {
      auto next = nodes[next_index].first;
      if (slippery || c < next)
        cout << name(c) << edge << name(next) << " [label=\"" << steps << "\"];\n";
    }
  cout << "}\n";
}

int trail_map::max_spanning_tree(visited v) const {
  // Kruskal
  int N = nodes.size();
  vector<int> link(N, 0);
  for (int i = 0; i < N; ++i)
    link[i] = i;
  auto find = [&](int i) {
                int i1 = i;
                while (link[i1] != i1)
                  i1 = link[i1];
                link[i] = i1;
                return i1;
              };
  auto onion = [&](int i, int j) {
                 if ((v & ((visited(1) << i) | (visited(1) << j))) != 0)
                   // At least one of these nodes has already been
                   // visited, so the edge can't be part of the tree
                   return false;
                 i = find(i);
                 j = find(j);
                 if (i == j)
                   // Not part of the spanning tree since it would
                   // complete a cycle
                   return false;
                 link[max(i, j)] = min(i, j);
                 // Edge was added to the tree
                 return true;
               };
  int max_steps = 0;
  for (auto [i, j, steps] : all_edges)
    if (onion(i, j))
      max_steps += steps;
  return max_steps;
}

int trail_map::longest_path() const {
  // Maximum possible length, node at tip of path, visited nodes,
  // steps so far
  using state = tuple<int, int, visited, int>;
  priority_queue<state> Q;
  auto add_state = [&](int n, visited v, int so_far) {
                     int max_rest = max_spanning_tree(v);
                     Q.push({so_far + max_rest, n, v, so_far});
                   };
  add_state(index(start), 0, 0);
  int the_end = index(finish);
  int best = 0;
  while (!Q.empty()) {
    auto [bound, n, v, so_far] = Q.top();
    Q.pop();
    if (bound < best)
      // None of the remaining partial paths can be better than what's
      // been found so far
      break;
    if (n == the_end) {
      // Reached the finish
      best = max(best, so_far);
      continue;
    }
    // Mark n as visited
    v |= visited(1) << n;
    // Check all non-visited successors
    for (auto [next, steps] : nodes[n].second)
    if ((v & (visited(1) << next)) == 0)
      add_state(next, v, so_far + steps);
  }
  return best;
}

void part1() { cout << trail_map(true).longest_path() << '\n'; }
void part2() { cout << trail_map(false).longest_path() << '\n'; }

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " partnum < input\n";
    exit(1);
  }
  if (*argv[1] == '1')
    part1();
  else
    part2();
  return 0;
}
