// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cassert>

using namespace std;

struct control {
  // Size of the platform
  int n;
  // The state of the platform
  vector<string> platform;

  control();

  enum { north = 0, west, south, east };
  // Tilt the platform in the direction indicated by orient
  void tilt(int orient = north);
  // Run a cycle north, west, south, east
  void cycle();

  // What's the load (always north-oriented)?
  int load() const;

  // Some random hash function for cycle detection
  size_t hash() const;
};

control::control() {
  string line;
  while (getline(cin, line)) {
    platform.push_back(line);
    assert(line.length() == platform.front().length());
  }
  // For simplicity I just did the square case, which is what the
  // input is
  assert(platform.front().length() == platform.size());
  n = platform.size();
}

void control::tilt(int orient) {
  // The algorithm is written just for "north", and the coordinate
  // mucking for orientation happens in the utility function at()
  auto at = [&](int r, int c) -> char & {
              switch (orient) {
              case north: return platform[r][c];
              case west: return platform[c][r];
              case south: return platform[n - 1 - r][c];
              default: return platform[c][n - 1 - r];
              }
            };
  // I initially had a list<int> for spaces where the rocks could roll
  // to, but the dynamic allocation for the list is the main time
  // sink.  Since there can't be more than n spaces in a column, I'll
  // just preallocate a vector of size n and pop_front / push_back in
  // that with a couple of pointers.
  vector<int> spaces(n, 0);
  for (int c = 0; c < n; ++c) {
    // Where are there free spaces in this column?
    int *spaces_start = &spaces[0];
    int *spaces_end = spaces_start;
    for (int r = 0; r < n; ++r) {
      char &what = at(r, c);
      if (what == '.')
        *spaces_end++ = r;
      else if (what == '#')
        // None shall pass!
        spaces_start = spaces_end;
      else if (spaces_start < spaces_end) {
        // Make like a rock and roll
        at(*spaces_start++, c) = 'O';
        // A new free space where the rock was
        what = '.';
        *spaces_end++ = r;
      }
    }
  }
}

void control::cycle() {
  for (int orient = north; orient <= east; ++orient)
    tilt(orient);
}

int control::load() const {
  int result = 0;
  for (int r = 0; r < n; ++r)
    for (int c = 0; c < n; ++c)
      if (platform[r][c] == 'O')
        result += n - r;
  return result;
}

size_t control::hash() const {
  size_t result = 0;
  int const nbits = 8 * sizeof(result);
  for (int r = 0; r < n; ++r)
    for (char c : platform[r]) {
      // Times some random prime, rotate, add next
      result *= 19650143;
      result = (result << (nbits - 3)) | (result >> 3);
      result += c;
    }
  return result;
}

void part1() {
  control ctrl;
  ctrl.tilt();
  cout << ctrl.load() << '\n';
}

void part2() {
  int const num_steps = 1000000000;
  control ctrl;
  map<size_t, int> seen;
  int step = 0;
  optional<int> cycle_length;
  while (step < num_steps) {
    ++step;
    ctrl.cycle();
    if (!cycle_length) {
      auto h = ctrl.hash();
      if (!seen.count(h))
        seen[h] = step;
      else {
        cycle_length = step - seen[h];
        int num_remaining = num_steps - step;
        int cycles_remaining = num_remaining / *cycle_length;
        step += cycles_remaining * *cycle_length;
      }
    }
  }
  cout << ctrl.load() << '\n';
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
