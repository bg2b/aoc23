// -*- C++ -*-
// g++ -std=c++17 -Wall -g -Ofast -o doit doit.cc
//
// This needs all the help it can get.  With stability = 2 and -Ofast,
// it's about a second.  If you're feeling lucky, you can reduce
// stability to 1 (which does work fine on my input), and then it's
// about half a second.
//
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <algorithm>
#include <optional>
#include <cassert>

using namespace std;

using coord = array<int, 2>;

coord dirs[4] = {coord{+1, 0}, coord{0, +1}, coord{-1, 0}, coord{0, -1}};

coord operator+(coord const &c1, coord const &c2) {
  return {c1[0] + c2[0], c1[1] + c2[1]};
}

struct farm {
  bool tiled;
  vector<string> garden;
  coord start;

  farm(bool tiled_);

  int size() const { return garden.size(); }
  char at(coord c) const;

  void step(vector<coord> &reached) const;
  // Do two steps from the coordinates in frontier.  This is useful
  // since two steps maintains stability in the set of reached plots.
  // prev is the previous value of frontier, and is used to avoid
  // going backwards into previously-visited areas.  Updates both prev
  // and frontier.  N.B., both prev and frontier *must* be in sorted
  // order, and that invariant is maintained.
  void step2(vector<coord> &prev, vector<coord> &frontier) const;
};

farm::farm(bool tiled_) : tiled(tiled_) {
  bool saw_start = false;
  string line;
  while (getline(cin, line)) {
    if (auto pos = line.find('S'); pos != string::npos) {
      assert(!saw_start);
      start = coord{int(pos), int(garden.size())};
      line[pos] = '.';
      saw_start = true;
    }
    garden.push_back(line);
    assert(line.length() == garden.front().length());
  }
  assert(saw_start);
  assert(garden.size() == garden.front().size());
}

char farm::at(coord c) const {
  c = c + start;
  int n = size();
  if (c[0] < 0 || c[0] >= n || c[1] < 0 || c[1] >= n) {
    if (!tiled)
      // Implicit wall in the finite case
      return '#';
    auto wrap = [=](int x) {
      x %= n;
      if (x < 0)
        x += n;
      return x;
    };
    c = {wrap(c[0]), wrap(c[1])};
  }
  return garden[c[1]][c[0]];
}

void farm::step(vector<coord> &reached) const {
  vector<coord> next;
  next.reserve(4 * reached.size());
  for (auto const &c : reached)
    for (auto const &dir : dirs)
      if (at(c + dir) != '#')
        next.push_back(c + dir);
  sort(next.begin(), next.end());
  next.erase(unique(next.begin(), next.end()), next.end());
  reached = move(next);
}

void farm::step2(vector<coord> &prev, vector<coord> &frontier) const {
  vector<coord> next;
  next.reserve(prev.size() + 4 * frontier.size());
  next.insert(next.end(), prev.begin(), prev.end());
  for (auto const &c : frontier)
    for (auto const &dir1 : dirs)
      if (at(c + dir1) != '#')
        for (auto const &dir2 : dirs)
          if (at(c + dir1 + dir2) != '#')
            next.push_back(c + dir1 + dir2);
  sort(next.begin(), next.end());
  next.erase(unique(next.begin(), next.end()), next.end());
  // Note that next includes new stuff but also things in *both* prev
  // and frontier; I don't want any of those.  Two set_differences and
  // a move will get everything into the right places...
  vector<coord> next_minus_prev;
  set_difference(next.begin(), next.end(), prev.begin(), prev.end(),
                 back_inserter(next_minus_prev));
  // The move also clears frontier...
  prev = move(frontier);
  set_difference(next_minus_prev.begin(), next_minus_prev.end(), prev.begin(),
                 prev.end(), back_inserter(frontier));
}

void part1() {
  farm frm(false);
  vector<coord> reached{coord{0, 0}};
  for (int _ = 0; _ < 64; ++_)
    frm.step(reached);
  cout << reached.size() << '\n';
}

// Extrapolate a series of values that should (eventually) be
// increasing as an exact quadratic.  There may be some initial
// transient though.  vals is the series of values observed so far.  x
// is the number of steps to extrapolate once a quadratic is found.
// Returns either the extrapolation value, or nullopt if it hasn't
// found the quadratic behavior yet.  See day 9 for the idea.
optional<size_t> quadratic(vector<size_t> const &vals, size_t x) {
  // Number of values at the second difference level that are required
  // to assume quadratic behavior
  size_t const stability = 2;
  if (vals.size() < stability + 2)
    // Not enough data
    return nullopt;
  auto diffs = [](vector<size_t> const &v) {
    vector<size_t> result;
    for (size_t i = 0; i + 1 < v.size(); ++i)
      result.push_back(v[i + 1] - v[i]);
    return result;
  };
  auto d1 = diffs(vals);
  auto d2 = diffs(d1);
  for (size_t i = 0; i < stability; ++i)
    if (d2[d2.size() - 1 - i] != d2.back())
      // No quadratic yet
      return nullopt;
  // The last d2 values have been constant; assume that's the
  // quadratic.  The last value will be increased by a linear number
  // of repetitions of the final d1 term and a sum-of-linear number of
  // repetitions of the final d2 term.
  return vals.back() + x * d1.back() + x * (x + 1) / 2 * d2.back();
}

void part2() {
  // Basic idea: there's a quadratically-growing diamond (or if the
  // tile is pathologically constrained, then possibly even linearly
  // growing or a constant) set of exactly-reachable things.  Just
  // find the quadratic and extrapolate (see day 9 for the
  // extrapolation approach).
  //
  // There are a few subtleties...
  //
  // I want the reachable set of plots to be stable, so I'm going two
  // steps at a time.  That makes the actual step process faster
  // anyway, since I can maintain a simple frontier of newly-reached
  // plots.  Since I do two steps each time, I have to start with the
  // right parity to get to the wanted number of steps.
  //
  // Also, if the tile happens to be an odd size (and it is in the
  // actual input), then to maintain parity in the sampled values, I'm
  // going to go 2*n steps between samples.
  //
  // If the input maliciously had an oddly-designed tile with a twisty
  // maze of little passages, it would probably be possible to fool
  // the quadratic detection.  I thought about doing some reasonable
  // number of initial steps to get it to a nontrivial set of
  // reachable plots, but eventually decided that it would probably be
  // better to just increase the stability setting in the detection if
  // that's a worry.
  //
  // The actual input is carefully designed so that a simpler approach
  // will also work.  There are clear paths from the start to the tile
  // edges which let the diamond expand unimpeded.  So for a given
  // copy of the tile, you can calculate exactly when the expanding
  // diamond will reach it, and from that you can do a search within
  // the single tile to see what would get covered after the desired
  // number of steps.  Since all of that sort of stuff gets repeated a
  // zillion times, you can cache everything, figure out the unique
  // cases, do some multiplies, and add everything up.  It'll probably
  // be much faster than the approach here.  I was initially going to
  // do that, but I had doubts in my ability to debug everything.
  // Note that the example input for part 2 does *not* have the nice
  // properties that would let this simpler method work.  Since the
  // approach here is general, I could check against the numbers in
  // the problem statement.
  farm frm(true);
  int n = frm.size();
  int const wanted_steps = 26501365;
  int steps = 0;
  vector<coord> reached{coord{0, 0}};
  // Get the parity right
  if (wanted_steps % 2 == 1) {
    frm.step(reached);
    ++steps;
  }
  // For efficiency, I only want to be stepping from a small(ish)
  // frontier, which has the coordinates that have to be searched.
  // prev_frontier holds the previous value of frontier.  It's used
  // when stepping to avoid going backwards; if a step from frontier
  // leads to prev_frontier, then it's already been seen.
  size_t reached_size = reached.size();
  vector<coord> frontier = reached;
  vector<coord> prev_frontier = reached;
  // Sampled sizes of the reachable sets
  vector<size_t> sizes;
  optional<size_t> ans;
  while (!ans.has_value()) {
    if (steps == wanted_steps)
      // Reached the required step count
      ans = reached_size;
    else if ((wanted_steps - steps) % (2 * n) == 0) {
      // Sample and look for a quadratic
      sizes.push_back(reached_size);
      ans = quadratic(sizes, (wanted_steps - steps) / (2 * n));
    }
    frm.step2(prev_frontier, frontier);
    reached_size += frontier.size();
    steps += 2;
  }
  cout << ans.value() << '\n';
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
