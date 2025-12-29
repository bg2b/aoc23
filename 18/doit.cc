// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <array>
#include <map>
#include <cassert>

// I pity the people who wrote part one with a flood fill.  Thanks be
// to <random diety> for having day 10 jog my memory...

using namespace std;

// int will break on part 2...
using coord = array<long, 2>;

map<char, coord> dirs = {{'U', coord{0, +1}},
                         {'D', coord{0, -1}},
                         {'L', coord{-1, 0}},
                         {'R', coord{+1, 0}}};

coord operator+(coord const &c1, coord const &c2) {
  return {c1[0] + c2[0], c1[1] + c2[1]};
}

coord operator*(long sc, coord const &c) { return {sc * c[0], sc * c[1]}; }

// Green's theorem gives us the area, and perimeter is trivial, but
// counting squares is a little subtle.  Consider R3,D3,L3,U3:
// +--+
// |..|
// |..|
// +--+
// perimeter = 3+3+3+3 = 12
// twice_area = 9*2 = 18
//
// The answer is (12+18)/2 + 1 = 16
//
// One way to see why this works is imagining the actual 3x3 rectangle
// described with integer grid points.  Put this on a discrete grid by
// pushing all the sides out by 1/2 (i.e., the corners of the discrete
// grid have half integer coordinates).  What's the resulting area?
// There was the initial area (from twice_area) as the interior.  Each
// unit on the perimeter pushing out gave another 1/2 square.  And the
// four corners where the perimeter bent gave another 4*(1/4) = 1.
// Introducing more bends in the perimeter doesn't bump the area more
// because a left turn takes away what the next right turn would give
// back.  Basically the 1 comes from one winding.

void solve(bool from_color) {
  coord pos{0, 0};
  long twice_area = 0;
  long perim = 0;
  auto update = [&](coord const &c) {
    twice_area += (c[0] + pos[0]) * (c[1] - pos[1]);
    pos = c;
  };
  char dir;
  int dist;
  string color;
  while (cin >> dir >> dist >> color) {
    if (from_color) {
      assert(color.length() == 9 && color.substr(0, 2) == "(#" &&
             color.back() == ')');
      dist = stoi(color.substr(2, 5), nullptr, 16);
      dir = "RDLU"[color[color.length() - 2] - '0'];
      assert(dirs.find(dir) != dirs.end());
    }
    perim += dist;
    update(pos + dist * dirs.at(dir));
  }
  update({0, 0});
  // Don't know the orientation
  twice_area = abs(twice_area);
  assert((perim + twice_area) % 2 == 0);
  cout << (perim + twice_area) / 2 + 1 << '\n';
}

void part1() { solve(false); }
void part2() { solve(true); }

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
