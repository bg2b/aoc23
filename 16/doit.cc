// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// part 2 is about 2 seconds without optimization, 0.2 seconds with
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <stdexcept>
#include <cassert>

using namespace std;

// Mathematical conventions are used: x coordinate ([0]) is
// horizontal, y coordinate positive is up on the screen, so lower
// left corner on the picture of the layout is (0, 0)

using coord = array<int, 2>;

coord operator+(coord const &c1, coord const &c2) {
  return {c1[0] + c2[0], c1[1] + c2[1]};
}

vector<coord> directions;

int find_dir(coord const &c) {
  for (size_t dir = 0; dir < directions.size(); ++dir)
    if (c == directions[dir])
      return dir;
  directions.push_back(c);
  return directions.size() - 1;
}

int const rt = find_dir({+1, 0});
int const up = find_dir({0, +1});
int const lt = find_dir({-1, 0});
int const dn = find_dir({0, -1});

using matrix = array<coord, 2>;

coord operator*(matrix const &m, coord const &c) {
  return {m[0][0] * c[0] + m[0][1] * c[1], m[1][0] * c[0] + m[1][1] * c[1]};
}

struct tile {
  char contents;
  char beam_dirs{0};

  tile(char c) : contents(c) {}

  bool energized() const { return beam_dirs != 0; }
  void reset() { beam_dirs = 0; }

  // Return the outgoing directions for a beam that is coming in from
  // direction dir.  But if dir has already been shot through this
  // tile, return {}.
  vector<int> shoot(int dir);

  // For my own sanity...
  static void verify();
};

vector<int> tile::shoot(int dir) {
  if (beam_dirs & (1 << dir))
    return {};
  beam_dirs |= 1 << dir;
  if (contents == '/')
    // Householder should be a household name...
    return {find_dir(matrix{coord{0, 1}, coord{1, 0}} * directions[dir])};
  if (contents == '\\')
    return {find_dir(matrix{coord{0, -1}, coord{-1, 0}} * directions[dir])};
  bool horizontal = directions[dir][0] != 0;
  if (contents == '|' && horizontal)
    return {up, dn};
  if (contents == '-' && !horizontal)
    return {rt, lt};
  return {dir};
}

void tile::verify() {
  assert(tile('/').shoot(rt) == vector<int>{up});
  assert(tile('/').shoot(lt) == vector<int>{dn});
  assert(tile('/').shoot(up) == vector<int>{rt});
  assert(tile('/').shoot(dn) == vector<int>{lt});
  assert(tile('\\').shoot(rt) == vector<int>{dn});
  assert(tile('\\').shoot(lt) == vector<int>{up});
  assert(tile('\\').shoot(up) == vector<int>{lt});
  assert(tile('\\').shoot(dn) == vector<int>{rt});
  vector<int> updown{up, dn};
  assert(tile('|').shoot(rt) == updown);
  assert(tile('|').shoot(lt) == updown);
  vector<int> rightleft{rt, lt};
  assert(tile('-').shoot(up) == rightleft);
  assert(tile('-').shoot(dn) == rightleft);
  assert(tile('.').shoot(up) == vector<int>{up});
  assert(tile('|').shoot(dn) == vector<int>{dn});
  assert(tile('-').shoot(rt) == vector<int>{rt});
  tile t('.');
  t.shoot(dn);
  assert(t.shoot(dn).empty());
}

struct cave {
  // The tiles in read order (i.e., layout[0] is the top row, which is
  // at y == height - 1)
  vector<vector<tile>> layout;

  cave();

  int width() const { return layout.front().size(); }
  int height() const { return layout.size(); }
  bool in_bounds(coord const &c) const;
  tile &at(coord const &c);

  // Shoot a light ray in at start in direction start_dir, return the
  // number of energized tiles
  int shoot(coord const &start, int start_dir);
};

cave::cave() {
  string line;
  while (cin >> line) {
    layout.push_back(vector<tile>());
    for (auto c : line)
      layout.back().emplace_back(tile(c));
    assert(layout.back().size() == layout.front().size());
  }
}

bool cave::in_bounds(coord const &c) const {
  return c[0] >= 0 && c[0] < width() && c[1] >= 0 && c[1] < height();
}

tile &cave::at(coord const &c) {
  return layout[height() - 1 - c[1]][c[0]];
}

int cave::shoot(coord const &start, int start_dir) {
  vector<pair<coord, int>> to_shoot;
  to_shoot.emplace_back(start, start_dir);
  while (!to_shoot.empty()) {
    auto [c, dir] = to_shoot.back();
    to_shoot.pop_back();
    if (!in_bounds(c))
      continue;
    auto next_dirs = at(c).shoot(dir);
    for (int next_dir : next_dirs)
      to_shoot.emplace_back(c + directions[next_dir], next_dir);
  }
  int result = 0;
  for (auto &row : layout)
    for (auto &t : row) {
      if (t.energized())
        ++result;
      t.reset();
    }
  return result;
}

void part1() {
  cave cv;
  cout << cv.shoot(coord{0, cv.height() - 1}, rt) << '\n';
}

void part2() {
  cave cv;
  int ans = 0;
  for (int x = 0; x < cv.width(); ++x) {
    ans = max(ans, cv.shoot({x, 0}, up));
    ans = max(ans, cv.shoot({x, cv.height() - 1}, dn));
  }
  for (int y = 0; y < cv.height(); ++y) {
    ans = max(ans, cv.shoot({0, y}, rt));
    ans = max(ans, cv.shoot({cv.width() - 1, y}, lt));
  }
  cout << ans << '\n';
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " partnum < input\n";
    exit(1);
  }
  tile::verify();
  if (*argv[1] == '1')
    part1();
  else
    part2();
  return 0;
}
