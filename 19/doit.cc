// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <map>
#include <cassert>

using namespace std;

// x, m, a, s
using part = array<int, 4>;

// index, test-for-<?, test value, if yes go to...
using test = tuple<int, bool, int, string>;

struct rule {
  vector<test> tests;
  string otherwise;
};

map<string, rule> rules;
vector<part> parts;

int index_of(char c) { return string("xmas").find(c); }

// Tedious but straightforward...
void read() {
  string line;
  while (getline(cin, line)) {
    if (line.empty())
      continue;
    if (line.front() == '{') {
      // A part
      auto val = [&](char const *what) {
        auto pos = line.find(what);
        assert(pos != string::npos);
        return atoi(line.c_str() + pos + 2);
      };
      part p{val("x="), val("m="), val("a="), val("s=")};
      parts.push_back(p);
      continue;
    }
    // A rule
    auto pos = line.find('{');
    assert(pos != string::npos);
    string name = line.substr(0, pos);
    line = line.substr(pos + 1);
    // Drop final }
    line.pop_back();
    while ((pos = line.find(',')) != string::npos) {
      // Part before ,
      string s = line.substr(0, pos);
      assert(s.length() > 2);
      int idx = index_of(s[0]);
      bool less = s[1] == '<';
      int v = atoi(s.c_str() + 2);
      auto dest_pos = s.find(':');
      assert(dest_pos != string::npos);
      string dest = s.substr(dest_pos + 1);
      assert(!dest.empty());
      rules[name].tests.emplace_back(idx, less, v, dest);
      // Advance past next ,
      line = line.substr(pos + 1);
    }
    assert(!line.empty());
    rules[name].otherwise = line;
  }
  assert(rules.count("in") != 0);
}

bool accept(part const &p) {
  string loc = "in";
  while (loc != "R" && loc != "A") {
    auto const &r = rules.at(loc);
    loc = r.otherwise;
    for (auto const &[idx, less, v, go_to] : r.tests)
      if ((less && p[idx] < v) || (!less && p[idx] > v)) {
        loc = go_to;
        break;
      }
  }
  return loc == "A";
}

// Could redo this in terms of part 2, but I'll leave it as
// this specialized form since it's clearer.
void part1() {
  int ans = 0;
  for (auto const &p : parts)
    if (accept(p))
      ans += p[0] + p[1] + p[2] + p[3];
  cout << ans << '\n';
}

// For part 2, I'm not going try to do any clever merging.  A range is
// just a pair of the lower and upper bounds for each of x, m, a, s.
// Rather than the usual C++ half-open convention, I'm using inclusive
// bounds since I think mentally it maps better to the problem
// statement.
using range = pair<part, part>;

// How many potential parts in a range?
long count(range const &rng) {
  auto const &[lower, upper] = rng;
  long result = 1;
  for (size_t i = 0; i < lower.size(); ++i)
    result *= max(upper[i] - lower[i] + 1, 0);
  return result;
}

// Is a range empty?
bool empty(range const &rng) { return count(rng) == 0; }

// Some generic empty range
range empty_rng{part{1, 1, 1, 1}, part{0, 0, 0, 0}};

// The function split takes a range and a test and splits the range
// into two parts: those which pass the test, and those which do not.
// Either of the parts may be empty.
pair<range, range> split(range const &rng, test const &t) {
  auto [idx, less, v, _] = t;
  if (!less)
    // Splitting a range for > v; instead split for < v + 1 and then
    // swap the parts at the end
    ++v;
  auto maybe_swap = [&](pair<range, range> const &result) {
    if (less)
      return result;
    return pair{result.second, result.first};
  };
  auto const &[lower, upper] = rng;
  if (upper[idx] < v)
    // Whole range is less than v, non-passing part is empty
    return maybe_swap({rng, empty_rng});
  if (v <= lower[idx])
    // Whole range is >= v, passing part is empty
    return maybe_swap({empty_rng, rng});
  assert(lower[idx] < v && v <= upper[idx]);
  // Cut at v
  range yes = rng;
  yes.second[idx] = v - 1;
  range no = rng;
  no.first[idx] = v;
  return maybe_swap({yes, no});
}

void part2() {
  // Things to process are pairs of (rule, range)
  vector<pair<string, range>> to_do;
  range all{part{1, 1, 1, 1}, part{4000, 4000, 4000, 4000}};
  to_do.emplace_back("in", all);
  long accepted = 0;
  while (!to_do.empty()) {
    auto [loc, rng] = to_do.back();
    to_do.pop_back();
    if (empty(rng) || loc == "R")
      // Nothing in the range, or all of these parts are rejects
      continue;
    if (loc == "A") {
      // This whole range is accepted
      accepted += count(rng);
      continue;
    }
    auto const &r = rules.at(loc);
    for (auto const &t : r.tests) {
      auto [yes, no] = split(rng, t);
      // Yes part goes to another rule
      to_do.emplace_back(get<3>(t), yes);
      // No part passes through remaining tests
      rng = no;
    }
    // The catch-all
    to_do.emplace_back(r.otherwise, rng);
  }
  cout << accepted << '\n';
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
