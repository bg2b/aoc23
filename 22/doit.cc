// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <vector>
#include <array>
#include <set>
#include <algorithm>

using namespace std;

// I'm using z as point[0] since it's more convenient for sorting by
// height above the ground
using point = array<int, 3>;

point min(point const &p1, point const &p2) {
  return {min(p1[0], p2[0]), min(p1[1], p2[1]), min(p1[2], p2[2])};
}

point max(point const &p1, point const &p2) {
  return {max(p1[0], p2[0]), max(p1[1], p2[1]), max(p1[2], p2[2])};
}

// Do two ranges [mn, mx] overlap?
bool overlaps(int mn1, int mx1, int mn2, int mx2) {
  return mx1 >= mn2 && mx2 >= mn1;
}

// A brick is an essential object
struct brick {
  // Lower and upper coordinates (inclusive)
  point ll;
  point ur;

  brick(string const &s);

  // Is this brick sitting on the ground?
  bool on_ground() const { return ll[0] == 1; }
  // Does this brick support brick b?
  bool supports(brick const &b) const;
  // Fall by the specified amount
  void fall(int z = 1) {
    ll[0] -= z;
    ur[0] -= z;
  }
};

brick::brick(string const &s) {
  stringstream ss(s);
  auto pt = [&]() -> point {
              int x, y, z;
              char comma;
              ss >> x >> comma >> y >> comma >> z;
              return {z, x, y};
            };
  auto p1 = pt();
  ss.ignore(1);
  auto p2 = pt();
  ll = min(p1, p2);
  ur = max(p1, p2);
}

bool brick::supports(brick const &b) const {
  if (ur[0] + 1 != b.ll[0])
    // b either hasn't reached here, or it's already fallen past
    return false;
  return (overlaps(ll[1], ur[1], b.ll[1], b.ur[1]) &&
          overlaps(ll[2], ur[2], b.ll[2], b.ur[2]));
}

bool operator<(brick const &b1, brick const &b2) {
  // All that really matters here is ordering by z
  if (b1.ll != b2.ll)
    return b1.ll < b2.ll;
  return b1.ur < b2.ur;
}

vector<brick> read() {
  vector<brick> result;
  string line;
  while (getline(cin, line))
    result.emplace_back(line);
  return result;
}

void solve(bool part2) {
  auto bricks = read();
  // Drop bricks starting with the lowest z
  sort(bricks.begin(), bricks.end());
  // Could probaby use one array and modify in place, but that's a
  // little more confusing
  vector<brick> pile;
  int max_z = 0;
  for (auto &b : bricks) {
    // Fast fall to just above the pile
    b.fall(max(b.ll[0] - (max_z + 1), 0));
    // Brick tetris
    while (!b.on_ground()) {
      // Heuristically it's probably a little faster to scan from the
      // back since those bricks are generally at higher z
      for (auto i = pile.rbegin(); i != pile.rend(); ++i)
        if (i->supports(b))
          // The horror...
          goto done_falling;
      b.fall();
    }
  done_falling:
    pile.push_back(b);
    max_z = max(max_z, b.ur[0]);
  }
  // supporters[i] => Who directly supports brick[i]?
  vector<vector<int>> supporters(bricks.size());
  for (size_t i = 0; i < pile.size(); ++i)
    for (size_t j = 0; j < i; ++j)
      if (pile[j].supports(pile[i]))
        supporters[i].push_back(j);
  int ans = 0;
  for (size_t i = 0; i < pile.size(); ++i) {
    // Count how many bricks would fall if brick[i] was disintegrated
    set<int> gone;
    // Zap
    gone.insert(i);
    // Note that someone sitting on the ground is always supported
    for (size_t j = i + 1; j < pile.size(); ++j)
      if (!pile[j].on_ground() &&
          all_of(supporters[j].begin(), supporters[j].end(),
                 [&](int s) { return gone.count(s) != 0; }))
        // Nothing supports brick[j], so it falls
        gone.insert(j);
    // The disintegrated brick doesn't count as fallen
    int num_that_fall = gone.size() - 1;
    ans += part2 ? num_that_fall : (num_that_fall == 0);
  }
  cout << ans << '\n';
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
