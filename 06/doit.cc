// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <vector>
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

// If I hold the button for t msec in a race of duration time msec,
// then the boat will travel t mm/msec for time-t msec, for a total
// distance of t*(time-t).  I want this to be greater than the record
// dist.
bool beats(num time, num dist, num t) { return t * (time - t) > dist; }

num transition(num time, num dist, num t1, num t2) {
  // t1 and t2 have different results.  Bisect [t1, t2] to find the
  // transition between winning and losing.  This is somewhat
  // overkill; just a linear search actually works fine given the
  // numbers involved.
  if (t2 == t1 + 1)
    return t1;
  bool beats1 = beats(time, dist, t1);
  bool beats2 = beats(time, dist, t2);
  assert(beats1 != beats2);
  num tm = (t1 + t2) / 2;
  bool beatsm = beats(time, dist, tm);
  if (beats1 != beatsm)
    return transition(time, dist, t1, tm);
  else
    return transition(time, dist, tm, t2);
}

num ways_to_win(num time, num dist) {
  // D_t t*(time-t) = (time-t)-t = time-2*t, which is 0 at time/2.
  // (If time is odd, then time/2 and time/2+1 are both equally good).
  // Anyway, if there's any win, then t = time/2 is one.
  num first_win = transition(time, dist, 0, time / 2) + 1;
  num last_win = transition(time, dist, time / 2, time);
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
