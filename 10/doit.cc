// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <cassert>

using namespace std;

using coord = pair<int, int>;

coord operator+(coord const &c1, coord const &c2) {
  return coord{c1.first + c2.first, c1.second + c2.second};
}

coord operator-(coord const &c1, coord const &c2) {
  return coord{c1.first - c2.first, c1.second - c2.second};
}

coord adjacent[4] = {{+1, 0}, {0, +1}, {-1, 0}, {0, -1}};

struct pipes {
  coord start;
  vector<string> maze;

  // Construct from cin
  pipes();

  int width() const { return maze.front().length(); }
  int height() const { return maze.size(); }
  char at(coord const &c) const;

  // If c1 is part of the loop, then is c2 also part of it?
  bool connected(coord const &c1, coord const &c2) const;

  // Distance from start to everything in the loop
  map<coord, int> dist_from_start() const;

  // Pitch pointless peripheral pipe pieces for pretty printing
  void clean_up(map<coord, int> const &dist);

  // How much space is inside the loop?
  int num_inside();
};

pipes::pipes() {
  string line;
  while (getline(cin, line)) {
    auto S = line.find('S');
    if (S != string::npos)
      start = {S, maze.size()};
    maze.push_back(line);
    assert(maze.front().length() == line.length());
  }
}

char pipes::at(coord const &c) const {
  if (c.first < 0 || c.first >= width())
    return '.';
  if (c.second < 0 || c.second >= height())
    return '.';
  return maze[c.second][c.first];
};

bool pipes::connected(coord const &c1, coord const &c2) const {
  char p1 = at(c1);
  assert(p1 != '.');
  char p2 = at(c2);
  if (p2 == '.')
    return false;
  auto is = [&](char p, char const *ok) -> bool {
              for (; *ok && p != *ok; ++ok)
                ;
              return *ok;
            };
  coord delta = c2 - c1;
  if (delta == pair{+1, 0} && is(p1, "S-FL") && is(p2, "-7J"))
    return true;
  if (delta == pair{-1, 0} && is(p1, "S-7J") && is(p2, "-FL"))
    return true;
  // N.B., second coordinate increasing means "going down"
  if (delta == pair{0, +1} && is(p1, "S|7F") && is(p2, "|JL"))
    return true;
  // Decreasing means "going up"
  if (delta == pair{0, -1} && is(p1, "S|JL") && is(p2, "|7F"))
    return true;
  return false;
}

map<coord, int> pipes::dist_from_start() const {
  map<coord, int> dist;
  dist[start] = 0;
  list<coord> to_search(1, start);
  while (!to_search.empty()) {
    assert(to_search.size() <= 2);
    coord c1 = to_search.front();
    to_search.pop_front();
    for (auto adj : adjacent) {
      coord c2 = c1 + adj;
      if (!dist.count(c2) && connected(c1, c2)) {
        dist[c2] = dist[c1] + 1;
        to_search.push_back(c2);
      }
    }
  }
  return dist;
}

void pipes::clean_up(map<coord, int> const &dist) {
  for (int j = 0; j < height(); ++j)
    for (int i = 0; i < width(); ++i)
      if (!dist.count({i, j}))
        maze[j][i] = '.';
}

int pipes::num_inside() {
  clean_up(dist_from_start());
  int result = 0;
  for (int j = 0; j < height(); ++j) {
    int crossings = 0;
    // crossing = number of vertical edges passed; an odd number means
    // I'm inside the loop.  One subtlety is that the bends have to be
    // consistent.
    for (int i = 0; i < width(); ++i) {
      char p = at({i, j});
      if (p == '.' && crossings % 2 == 1)
        ++result;
      else if (p == '|' || p == 'L' || p == 'J')
        ++crossings;
    }
  }
  return result;
}

void part1() {
  int ans = 0;
  for (auto const &[_, d] : pipes().dist_from_start())
    ans = max(ans, d);
  cout << ans << '\n';
}

void part2() { cout << pipes().num_inside() << '\n'; }

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
