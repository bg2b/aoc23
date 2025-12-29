// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>

using namespace std;

void part1() {
  // Collect edges
  vector<string> names;
  auto index_of = [&](string const &comp) -> unsigned {
    if (auto p = find(names.begin(), names.end(), comp); p != names.end())
      return p - names.begin();
    names.push_back(comp);
    return names.size() - 1;
  };
  vector<pair<unsigned, unsigned>> edges;
  unsigned lhs = 0;
  string comp;
  while (cin >> comp)
    if (comp.back() == ':') {
      comp.pop_back();
      lhs = index_of(comp);
    } else {
      unsigned i = index_of(comp);
      edges.emplace_back(min(lhs, i), max(lhs, i));
    }
  unsigned n = names.size();
  assert(n >= 3);
  // Knowing that there are three cross edges means that random
  // Kruskal will have a good chance of finding the min cut
  default_random_engine g;
  while (true) {
    shuffle(edges.begin(), edges.end(), g);
    // Onion-find
    vector<unsigned> link(n, 0);
    for (unsigned i = 0; i < n; ++i)
      link[i] = i;
    auto find = [&](unsigned i) {
      unsigned i1 = i;
      while (link[i1] != i1)
        i1 = link[i1];
      link[i] = i1;
      return i1;
    };
    unsigned num_linked = 0;
    auto link_if_disjoint = [&](unsigned i, unsigned j) {
      if (i == j)
        return false;
      if (i > j)
        swap(i, j);
      link[j] = i;
      ++num_linked;
      // Are there two components left?
      return num_linked == n - 2;
    };
    // Link until there are two components remaining
    for (auto [i, j] : edges)
      if (link_if_disjoint(find(i), find(j)))
        break;
    // Are there three cross edges?
    unsigned num_spanning = 0;
    for (auto [i, j] : edges)
      if (find(i) != find(j))
        ++num_spanning;
    if (num_spanning == 3) {
      unsigned part1_size = 0;
      for (unsigned i = 0; i < n; ++i)
        if (find(i) == find(0))
          ++part1_size;
      cout << part1_size * (n - part1_size) << '\n';
      break;
    }
  }
}

void part2() { cout << "Push The Big Red Button\n"; }

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
