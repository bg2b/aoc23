// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit1 doit1.cc
// About 0.5 seconds for part 2 without optimization, under 0.1
// seconds with
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <set>
#include <cassert>

using namespace std;

// A couple of hacks for speed:
// 1. Reverse search from factory without movement restrictions to get
//    a reasonable heuristic distance to solution
// 2. State encoding/decoding to pack states into ints
// 3. Main search uses static allocation of min losses and sort-of for
//    priority queue, based on #2
// Ugly, but effective...

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
  // Bounds for min loss to reach the factory with unrestricted
  // movement
  vector<vector<int>> unrestricted;

  city(int min_steps_, int max_steps_);

  int width() const { return heat_loss.front().length(); }
  int height() const { return heat_loss.size(); }
  bool in_bounds(coord const &c) const;
  int loss(coord const &c) const { return heat_loss[c[1]][c[0]] - '0'; }

  // Simple Dijklmnopqrstra to get unrestricted movement bounds
  void compute_unrestricted();
  int min_to_factory(coord const &c) const { return unrestricted[c[1]][c[0]]; }

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
  compute_unrestricted();
}

bool city::in_bounds(coord const &c) const {
  return c[0] >= 0 && c[0] < width() && c[1] >= 0 && c[1] < height();
}

void city::compute_unrestricted() {
  unrestricted.resize(height(), vector<int>(width(), 10 * width() * height()));
  auto at = [&](coord const &c) -> int & { return unrestricted[c[1]][c[0]]; };
  // Priority queue (not really a queue, but whatevs)
  auto ll = [&](coord const &c1, coord const &c2) {
              int d1 = at(c1);
              int d2 = at(c2);
              if (d1 != d2)
                return d1 < d2;
              return c1 < c2;
            };
  set<coord, decltype(ll)> Q(ll);
  // Update the loss for a state; if less than the previous loss to
  // get to the state, update Q
  auto update = [&](coord const &c, int loss) {
                  if (loss >= at(c))
                    return;
                  if (auto p = Q.find(c); p != Q.end())
                    Q.erase(p);
                  at(c) = loss;
                  Q.insert(c);
                };
  update(factory, 0);
  while (!Q.empty()) {
    auto c = *Q.begin();
    int total_loss = at(c);
    Q.erase(Q.begin());
    for (auto const &dir : dirs) {
      auto nc = c + dir;
      if (in_bounds(nc))
        update(nc, total_loss + loss(nc));
    }
  }
}

int city::min_loss() const {
  int const h = height();
  int const w = width();
  // 10*w*h is guaranteed to be larger than any real distance
  int const infinity = 10 * w * h;
  // Coordinate, direction index, steps before being allowed to turn
  //
  // Note that I've written things so that at least one step must be
  // taken in the given direction.  I found this a little less
  // confusing overall, but am not really convinced it's best.  It
  // does need some way of knowing which way it's been moving.
  using state = tuple<coord, int, int>;
  // Encode a state
  auto encode = [&](state const &s) {
                  auto [c, dir, steps_to_turn] = s;
                  int index = c[0];
                  index *= h;
                  index += c[1];
                  index *= max_steps + 1;
                  index += steps_to_turn;
                  index *= 4;
                  index += dir;
                  return index;
                };
  // Decode
  auto decode = [&](int enc) {
                  int dir = enc & 0x3;
                  enc >>= 2;
                  int steps_to_turn = enc % (max_steps + 1);
                  enc /= max_steps + 1;
                  int c1 = enc % h;
                  enc /= h;
                  int c0 = enc;
                  return state{coord{c0, c1}, dir, steps_to_turn};
                };
  // Quick check on the encoding
  state rand{coord{w / 2, h - 1}, 2, max_steps - 2};
  auto checkrand = decode(encode(rand));
  assert(rand == checkrand);
  // Minimum losses (so far) to reach the given state.  Everything's
  // been packed up into a vector to avoid allocation.
  int num_states = encode({coord{w - 1, h - 1}, 3, max_steps}) + 1;
  vector<int> so_far(num_states, infinity);
  auto at_enc = [&](int enc) -> int & { return so_far[enc]; };
  // The encoded form of a state to be explored stuck is onto Q[cost]
  vector<vector<int>> Q;
  // Update the loss for a state; if less than the previous loss to
  // get to the state, update Q.  nextQ is the current minimum
  // (estimated) loss that I'm looking at, i.e., the state being
  // explored came from Q[nextQ].
  unsigned nextQ = 0;
  auto update = [&](state const &s, int loss) {
                  int e = encode(s);
                  int &current = at_enc(e);
                  if (loss >= current)
                    return;
                  if (current < infinity) {
                    // Remove from Q.  Can get away without this and
                    // it might even be always OK (and it does seem
                    // epsilon faster), but it feels iffy...
                    unsigned qpos = current + min_to_factory(get<0>(s));
                    assert(qpos < Q.size());
                    // Ugh...
                    auto p = find(Q[qpos].begin(), Q[qpos].end(), e);
                    if (p != Q[qpos].end())
                      Q[qpos].erase(p);
                  }
                  current = loss;
                  unsigned qpos = loss + min_to_factory(get<0>(s));
                  if (qpos >= Q.size())
                    // Need more space in Q
                    Q.resize(qpos + 100);
                  Q[qpos].push_back(e);
                  // It's possible that we've found a faster way to a
                  // state and that nextQ has already advanced past
                  // qpos; back up if needed
                  nextQ = min(nextQ, qpos);
                };
  // Can start either direction
  update({{0, 0}, 0, max_steps}, 0);
  update({{0, 0}, 1, max_steps}, 0);
  while (true) {
    while (nextQ < Q.size() && Q[nextQ].empty())
      ++nextQ;
    assert(nextQ < Q.size());
    int enc = Q[nextQ].back();
    Q[nextQ].pop_back();
    auto s = decode(enc);
    int total_loss = at_enc(enc);
    auto [c, dir_idx, steps_to_turn] = s;
    auto dir = dirs[dir_idx];
    if (c == factory)
      return total_loss;
    // Next movement step
    c = c + dir;
    assert(in_bounds(c));
    total_loss += loss(c);
    --steps_to_turn;
    assert(steps_to_turn >= 0);
    // Successors
    for (unsigned next_dir_idx = 0; next_dir_idx < 4; ++next_dir_idx) {
      auto next_dir = dirs[next_dir_idx];
      if (next_dir == -1 * dir)
        // Can't reverse direction
        continue;
      if (next_dir == dir) {
        // Not turning
        if (steps_to_turn > 0 && in_bounds(c + next_dir))
          // A legal step
          update({c, next_dir_idx, steps_to_turn}, total_loss);
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
      update({c, next_dir_idx, max_steps}, total_loss);
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
