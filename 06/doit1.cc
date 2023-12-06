// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit1 doit1.cc
// ./doit1 1 < input  # part 1
// ./doit1 2 < input  # part 2

#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

using namespace std;

using num = long;

vector<num> read1() {
  string _;
  cin >> _;
  vector<num> thing;
  num n;
  while (cin >> n)
    thing.push_back(n);
  cin.clear();
  return thing;
}

vector<num> times;
vector<num> dists;

void read() {
  times = read1();
  dists = read1();
  assert(times.size() == dists.size());
}

// t is very close to the transition between winning and losing.  Find
// the winning t that is exactly at the boundary.  edge = -1 to find
// the first win and +1 to find the last.
num transition(num time, num dist, num t, num edge) {
  auto winning = [&](num t1) { return t1 * (time - t1) > dist; };
  while (!winning(t))
    t -= edge;
  while (winning(t + edge))
    t += edge;
  return t;
}

num ways_to_win(num time, num dist) {
  // When does t*(time-t) = dist?  Expand: t^2-time*t+dist = 0.
  // Quadratic formula says (time +/- sqrt(time^2-4*dist)) / 2.  The -
  // is for the first loss/win transition, the + is for the second.
  num disc = time * time - 4 * dist;
  assert(disc >= 0);
  num approx_sqrt_disc(sqrt(disc));
  // I don't have enough spare brain cells to try to ensure perfection
  // in the face of whatever rounding might happen, so just tweak
  // slightly if necessary.
  num first_win = transition(time, dist, (time - approx_sqrt_disc) / 2, -1);
  num last_win = transition(time, dist, (time + approx_sqrt_disc) / 2, +1);
  return last_win - first_win + 1;
}

void solve() {
  num ans = 1;
  for (size_t i = 0; i < times.size(); ++i)
    ans *= ways_to_win(times[i], dists[i]);
  cout << ans << '\n';
}

void part1() { solve(); }

void concat(vector<num> &thing) {
  string catenated;
  for (auto n : thing)
    catenated += to_string(n);
  thing = vector(1, stol(catenated));
}

void part2() {
  concat(times);
  concat(dists);
  solve();
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " partnum < input\n";
    exit(1);
  }
  read();
  if (*argv[1] == '1')
    part1();
  else
    part2();
  return 0;
}
