// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit1 doit1.cc
// ./doit1 1 < input  # part 1
// ./doit1 2 < input  # part 2

// Much better than the explicit cache in doit.cc, using the
// observation that there's no need to be modifying the
// representations of the condition or the groups.  Just a couple of
// integer indexes suffices.

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cassert>

using namespace std;

// conds index, group index
using config = pair<unsigned, unsigned>;

string conds;
vector<unsigned> groups;
map<config, size_t> ways;

// Number of ways to match conds[ci, end) against groups[gi, end)
size_t all_ways(unsigned ci, unsigned gi) {
  if (ci >= conds.length())
    // Nothing else in conds, are we at the end of groups?
    return gi == groups.size() ? 1 : 0;
  // Utility functions to process an empty space or a group
  auto empty = [&]() -> size_t { return all_ways(ci + 1, gi); };
  auto group =
    [&]() -> size_t {
      if (gi == groups.size())
        // No groups left
        return 0;
      unsigned num_in_group = groups[gi];
      if (conds.length() - ci < num_in_group)
        // The group is too long
        return 0;
      // The next num_in_group must all be #
      for (unsigned i = ci + 1; i < ci + num_in_group; ++i)
        if (conds[i] == '.')
          // Group would end too soon
          return 0;
      if (ci + num_in_group < conds.size() && conds[ci + num_in_group] == '#')
        // Group could not end at the right place
        return 0;
      // Consistent
      return all_ways(ci + num_in_group + 1, gi + 1);
    };
  if (conds[ci] == '.')
    // Skip definite spaces
    return empty();
  if (conds[ci] == '#')
    // The next group must start here
    return group();
  // Decision point, check cache
  if (auto p = ways.find({ci, gi}); p != ways.end())
    return p->second;
  // Count both ways of setting the last ?
  return ways[{ci, gi}] = empty() + group();
}

void solve(int unfoldings) {
  string line;
  size_t ans = 0;
  while (getline(cin, line)) {
    stringstream ss(line);
    ss >> conds;
    groups = vector<unsigned>(1, 0);
    char comma;
    while (ss >> groups.back() >> comma)
      groups.push_back(0);
    conds.push_back('?');
    // Unfold
    auto unfolded_conds = conds;
    auto unfolded_groups(groups);
    for (int _ = 1; _ < unfoldings; ++_) {
      unfolded_conds += conds;
      unfolded_groups.insert(unfolded_groups.end(),
                             groups.begin(), groups.end());
    }
    unfolded_conds.pop_back();
    conds = unfolded_conds;
    groups = unfolded_groups;
    ways.clear();
    ans += all_ways(0, 0);
  }
  cout << ans << '\n';
}

void part1() { solve(1); }
void part2() { solve(5); }

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
