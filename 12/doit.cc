// -*- C++ -*-
// g++ -std=c++17 -Wall -g -O -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2
// -O cuts part 2 down to under a second, just over a second otherwise

// See much less silly solution in doit1.cc

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>

using namespace std;

// Minimum number of groups for conds; slightly subtle, so be careful
size_t min_grps(string const &conds) {
  size_t result = 0;
  bool in_group = false;
  for (char c : conds)
    if (c == '#' && !in_group) {
      // Group must start; want it to be as big as possible
      ++result;
      in_group = true;
    } else if (c == '.')
      // Group must end
      in_group = false;
  return result;
}

// Maximum number of groups for conds; slightly subtle, so be careful
size_t max_grps(string const &conds) {
  size_t result = 0;
  bool in_group = false;
  for (char c : conds)
    if (c != '.' && !in_group) {
      // Start group eagerly, then try to cut it off as quickly as possible
      ++result;
      in_group = true;
    } else if (c != '#')
      // Group could end
      in_group = false;
  return result;
}

// Cut off known stuff from the end of conds and groups.  Return
// whether what's left might be consistent
bool consistent(string &conds, vector<int> &groups) {
  while (!conds.empty()) {
    if (conds.back() == '.') {
      // Drop empty space
      conds.pop_back();
      continue;
    }
    // Scan last group
    int len = conds.length();
    int p = len;
    bool any_unknown = false;
    int min_last = 0;
    while (p > 0 && conds[p - 1] != '.') {
      --p;
      if (conds[p] == '?')
        any_unknown = true;
      if (!any_unknown)
        ++min_last;
    }
    if (min_last > 0 && (groups.empty() || groups.back() < min_last))
      // There's definitely some last group, and it's too big
      return false;
    if (any_unknown)
      // Last part has ?'s and could be one group; just check sizes
      return min_grps(conds) <= groups.size() && groups.size() <= max_grps(conds);
    // Looking at a real group
    assert(conds[p] == '#' && !groups.empty());
    if (len - p != groups.back())
      // Wrong length
      return false;
    // Chop off the match
    groups.pop_back();
    conds.erase(conds.begin() + p, conds.end());
  }
  // Ate the whole thing
  return groups.empty();
}

using config = pair<string, vector<int>>;

size_t all_ways(string conds, vector<int> groups, map<config, size_t> &ways) {
  // Base cases
  if (!consistent(conds, groups))
    return 0;
  if (conds.empty())
    return 1;
  // Was this already checked?
  config key{conds, groups};
  auto p = ways.find(key);
  if (p != ways.end())
    return p->second;
  // Count both ways of setting the last ?
  auto &ans = ways[key];
  auto unknown = conds.rfind('?');
  assert(unknown != string::npos);
  conds[unknown] = '.';
  ans += all_ways(conds, groups, ways);
  conds[unknown] = '#';
  ans += all_ways(conds, groups, ways);
  return ans;
}

void solve(int unfoldings) {
  string line;
  size_t ans = 0;
  while (getline(cin, line)) {
    stringstream ss(line);
    string conds;
    ss >> conds;
    vector<int> groups(1, 0);
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
    // Initial trim from one end
    if (!consistent(conds, groups))
      // Never happens
      continue;
    reverse(conds.begin(), conds.end());
    reverse(groups.begin(), groups.end());
    // Search from other
    map<config, size_t> ways;
    ans += all_ways(conds, groups, ways);
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
