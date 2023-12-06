// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <limits>
#include <vector>
#include <cassert>

using namespace std;

// All ranges are usual C++-style half-open [start, end)

using type = long;

type inf = numeric_limits<type>::max();

// source start => [dest start, dest_end)
using mapping = map<type, pair<type, type>>;

// All the types of seeds
vector<type> seeds;
// Source type => dest type and mapping between their ranges
map<string, pair<string, mapping>> mappings;

void read() {
  string line;
  while (getline(cin, line)) {
    if (line.empty())
      continue;
    if (seeds.empty()) {
      // First line
      stringstream ss(line.substr(line.find(' ') + 1));
      type seed;
      while (ss >> seed)
        seeds.push_back(seed);
    } else {
      // Some map
      string source = line.substr(0, line.find('-'));
      string dest = line.substr(line.rfind('-') + 1);
      dest = dest.substr(0, dest.find(' '));
      mappings[source].first = dest;
      auto &src2dst = mappings[source].second;
      type dest_start, source_start, length;
      while (cin >> dest_start >> source_start >> length) {
        assert(length > 0);
        src2dst[source_start] = {dest_start, dest_start + length};
      }
      cin.clear();
    }
  }
}

// What's the minimum location that things of type source in the range
// [start, end) map to?
type min_loc(string const &source, type start, type end) {
  type result = inf;
  if (start >= end)
    // Empty range
    return result;
  if (source == "location")
    // All mapping done, start is the minimum
    return start;
  assert(mappings.count(source) > 0);
  auto const &[dest, src2dst] = mappings[source];
  // p points just beyond anything that might map [start, end)
  auto p = src2dst.upper_bound(end - 1);
  while (p != src2dst.begin() && start < end) {
    --p;
    type source_start = p->first;
    auto [dest_start, dest_end] = p->second;
    type source_end = source_start + (dest_end - dest_start);
    type offset = dest_start - source_start;
    // Next unmapped part
    type unmapped_start = max(start, source_end);
    result = min(result, min_loc(dest, unmapped_start, end));
    end = min(end, source_end);
    if (start >= end)
      break;
    // Next mapped part
    type mapped_start = max(start, source_start);
    assert(mapped_start < end && end <= source_end);
    result = min(result, min_loc(dest, mapped_start + offset, end + offset));
    end = min(end, source_start);
  }
  // Any remaining unmapped part
  return min(result, min_loc(dest, start, end));
}

vector<pair<type, type>> seed_ranges;

void part1() {
  for (auto seed : seeds)
    seed_ranges.emplace_back(seed, seed + 1);
}

void part2() {
  for (size_t i = 0; i < seeds.size(); i += 2)
    seed_ranges.emplace_back(seeds[i], seeds[i] + seeds[i + 1]);
}

void solve() {
  type result = inf;
  for (auto [start, end] : seed_ranges)
    result = min(result, min_loc("seed", start, end));
  cout << result << '\n';
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
  solve();
  return 0;
}
