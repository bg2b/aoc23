// -*- C++ -*-
// g++ -std=c++17 -Wall -g -O -o doit doit.cc
// Optimization is helpful for part 2, about 3.5 seconds without, 0.5
// second with
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
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

// This is just brute force, which for part 2 is still in the range of
// working provided that the search is reasonably optimized.
//
// I tried a simple reachability search as a pruning heuristic (can
// the finish still be reached?), but it seemed to just make things
// slower.  It searches so fast that whatever it gains from that
// pruning is lost in the extra overhead of each step.  Something more
// clever is probably required.

// List of successor node index, number of steps to get there
using edges = vector<pair<int, int>>;
// Coordinate of the node, what it connects to
using node = pair<coord, edges>;
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

  // Return the longest path to the finish from n, given the number of
  // steps taken so far and a bit set representation of what's been
  // visited
  int longest_path(int n, int so_far, visited v) const;
  int longest_path() const { return longest_path(index(start), 0, 0); }
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

int trail_map::longest_path(int n, int so_far, visited v) const {
  // Add n to the set of visited nodes
  v |= visited(1) << n;
  while (true) {
    if (n == 1)
      // The finish
      return so_far;
    int result = -1;
    for (auto [next, steps] : nodes[n].second)
      if ((v & (visited(1) << next)) == 0)
        // Max over unvisited successors
        result = max(result, longest_path(next, so_far + steps, v));
    return result;
  }
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
