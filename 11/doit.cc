// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <cassert>

using namespace std;

using coord = pair<int, int>;

struct the_final_frontier {
  set<coord> galaxies;
  // Distances from {0, 0}
  vector<int> dr;
  vector<int> dc;

  the_final_frontier(int expansion);

  // Cumulative distances from 0 along dimension dim
  vector<int> delta(int coord::*dim, int expansion) const;

  int distance(coord const &g1, coord const &g2) const;
  long total_distance() const;
};

the_final_frontier::the_final_frontier(int expansion) {
  string line;
  int row = 0;
  while (getline(cin, line)) {
    for (size_t col = 0; col < line.length(); ++col)
      if (line[col] == '#')
        galaxies.insert({row, col});
    ++row;
  }
  dr = delta(&coord::first, expansion);
  dc = delta(&coord::second, expansion);
}

vector<int> the_final_frontier::delta(int coord::*dim, int expansion) const {
  set<int> nonempty;
  for (auto const &g : galaxies)
    nonempty.insert(g.*dim);
  // How could something nonempty ever be empty?  Why am I bothering
  // to check this?
  assert(!nonempty.empty());
  vector<int> result(1, 0);
  int max_d = *prev(nonempty.end());
  for (int d = 0; d < max_d; ++d)
    result.push_back(result.back() + (nonempty.count(d) ? 1 : expansion));
  return result;
}

int the_final_frontier::distance(coord const &g1, coord const &g2) const {
  // Since passing through galaxies is OK, this decomposes
  return abs(dr[g2.first] - dr[g1.first]) + abs(dc[g2.second] - dc[g1.second]);
}

long the_final_frontier::total_distance() const {
  long ans = 0;
  for (auto g1 = galaxies.begin(); g1 != galaxies.end(); ++g1)
    for (auto g2 = next(g1); g2 != galaxies.end(); ++g2)
      ans += distance(*g1, *g2);
  return ans;
}

void part1() { cout << the_final_frontier(2).total_distance() << '\n'; }
void part2() { cout << the_final_frontier(1000000).total_distance() << '\n'; }

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
