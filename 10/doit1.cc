// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit1 doit1.cc
// ./doit1 1 < input  # part 1
// ./doit1 2 < input  # part 2

// Alternative version based on discussion with Footkick72

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

using namespace std;

using coord = pair<int, int>;

coord operator+(coord const &c1, coord const &c2) {
  return coord{c1.first + c2.first, c1.second + c2.second};
}

coord operator-(coord const &c1, coord const &c2) {
  return coord{c1.first - c2.first, c1.second - c2.second};
}

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
  // Find the two coordinates adjacent to c
  vector<coord> adjacent_to(coord const &c) const;

  // Find the loop
  vector<coord> loop() const;
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
  if (p2 == 'S')
    return connected(c2, c1);
  auto is = [](char p, string const &ok) { return ok.find(p) != string::npos; };
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

vector<coord> pipes::adjacent_to(coord const &c) const {
  vector<coord> result;
  for (auto adj : {coord{+1, 0}, coord{0, +1}, coord{-1, 0}, coord{0, -1}})
    if (connected(c, c + adj))
      result.push_back(c + adj);
  return result;
}

vector<coord> pipes::loop() const {
  coord next = adjacent_to(start).back();
  vector<coord> result(1, start);
  while (next != start) {
    coord prev = result.back();
    result.push_back(next);
    auto next_next = adjacent_to(next);
    next = next_next.front() == prev ? next_next.back() : next_next.front();
  }
  return result;
}

void part1() { cout << pipes().loop().size() / 2 << '\n'; }

void part2() {
  auto loop = pipes().loop();
  // Area of loop by Green's theorem
  int twice_area = 0;
  auto prev = loop.back();
  for (auto const &c : loop) {
    twice_area += (c.first + prev.first) * (c.second - prev.second);
    prev = c;
  }
  // The loop orientation is random, so area could be negative
  twice_area = abs(twice_area);
  // Answer by Pick's theorem
  cout << (twice_area - loop.size()) / 2 + 1 << '\n';
}

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
