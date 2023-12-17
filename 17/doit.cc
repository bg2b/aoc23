// -*- C++ -*-
// g++ -std=c++17 -Wall -g -O -o doit doit.cc
// This is not overly fast, a bit over a second for part 2 even with
// optimization
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <set>
#include <cassert>

using namespace std;

// This needs a more clever search, e.g., A*.  Perhaps reverse
// shortest path from the factory with no movement restrictions and
// use that as the bound?

using coord = array<int, 2>;

coord dirs[4] = {coord{+1, 0}, coord{0, +1}, coord{-1, 0}, coord{0, -1}};

coord operator+(coord const &c1, coord const &c2) {
  return {c1[0] + c2[0], c1[1] + c2[1]};
}

coord operator*(int sc, coord const &c) { return {sc*c[0], sc*c[1]}; }

struct city {
  // Minimum and maximum required moves of a crucible
  int min_steps;
  int max_steps;
  // The city layout
  vector<string> heat_loss;
  // Where the factor is located
  coord factory;

  city(int min_steps_, int max_steps_);

  int width() const { return heat_loss.front().length(); }
  int height() const { return heat_loss.size(); }
  bool in_bounds(coord const &c) const;
  int loss(coord const &c) const { return heat_loss[c[1]][c[0]] - '0'; }

  // Compute minimum loss from start to factory, given the movement
  // restrictions
  int min_loss() const;
};

city::city(int min_steps_, int max_steps_) :
  min_steps(min_steps_), max_steps(max_steps_) {
  string line;
  while (getline(cin, line)) {
    heat_loss.push_back(line);
    assert(line.length() == heat_loss.front().length());
  }
  factory = {width() - 1, height() - 1};
}

bool city::in_bounds(coord const &c) const {
  return c[0] >= 0 && c[0] < width() && c[1] >= 0 && c[1] < height();
}

// Dijklmnopqrstra is our lord and savior...
int city::min_loss() const {
  // Coordinate, direction, steps before being allowed to turn
  //
  // Note that I've written things so that at least one step must be
  // taken in the given direction.  I found this a little less
  // confusing overall, but am not really convinced it's best.  It
  // does need some way of knowing which way it's been moving.
  using state = tuple<coord, coord, int>;
  // Minimum losses (so far) to reach the given state.  Normally I'd
  // be lazy and use a map, but this is one case where it actually
  // matters for the speed...
  auto hash = [](state const &s) {
                auto [c, _, steps_to_turn] = s;
                return (c[0] * 1000 + c[1]) * 1000 + steps_to_turn;
              };
  size_t sz_guess = 5 * width() * height();
  unordered_map<state, int, decltype(hash)> so_far(sz_guess, hash);
  // Priority queue (not really a queue, but whatevs)
  auto ll = [&](state const &s1, state const &s2) {
              int d1 = so_far.at(s1);
              int d2 = so_far.at(s2);
              if (d1 != d2)
                // Order by loss, so upon getting to the factory, it's
                // guaranteed to be the minimum loss
                return d1 < d2;
              // Any random order here
              return s1 < s2;
            };
  set<state, decltype(ll)> Q(ll);
  // Update the loss for a state; if less than the previous loss to
  // get to the state, update Q
  auto update = [&](state const &s, int loss) {
                  if (auto p = so_far.find(s); p != so_far.end()) {
                    if (loss >= p->second)
                      // Was no better
                      return;
                    // Be sure to remove from Q before mucking with
                    // the order
                    if (auto p = Q.find(s); p != Q.end())
                      Q.erase(p);
                  }
                  so_far.insert_or_assign(s, loss);
                  Q.insert(s);
                };
  // Can start either direction
  update({{0, 0}, {+1, 0}, max_steps}, 0);
  update({{0, 0}, {0, +1}, max_steps}, 0);
  while (true) {
    assert(!Q.empty());
    auto s = *Q.begin();
    int total_loss = so_far.at(s);
    auto [c, dir, steps_to_turn] = s;
    Q.erase(Q.begin());
    if (c == factory)
      return total_loss;
    // Next movement step
    c = c + dir;
    assert(in_bounds(c));
    total_loss += loss(c);
    --steps_to_turn;
    assert(steps_to_turn >= 0);
    // Successors
    for (auto const &next_dir : dirs) {
      if (next_dir == -1 * dir)
        // Can't reverse direction
        continue;
      if (next_dir == dir) {
        // Not turning
        if (steps_to_turn > 0 && in_bounds(c + next_dir))
          // A legal step
          update({c, next_dir, steps_to_turn}, total_loss);
        continue;
      }
      // Turning left or right
      if (steps_to_turn > max_steps - min_steps)
        // Cannot turn yet
        continue;
      if (!in_bounds(c + min_steps * next_dir))
        // No stepping out of bounds (also enforces that when reaching
        // the factory, the crucible must be able to turn, which is a
        // subtle requirement in the problem)
        continue;
      update({c, next_dir, max_steps}, total_loss);
    }
  }
}

void part1() { cout << city(1, 3).min_loss() << '\n'; }
void part2() { cout << city(4, 10).min_loss() << '\n'; }

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
