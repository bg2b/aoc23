// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

using namespace std;

int extrapolate(vector<int> ns) {
  if (all_of(ns.begin(), ns.end(), [](int n) { return n == 0; }))
    return 0;
  vector<int> diffs;
  for (size_t i = 0; i + 1 < ns.size(); ++i)
    diffs.push_back(ns[i + 1] - ns[i]);
  return ns.back() + extrapolate(diffs);
}

void solve(bool backwards) {
  int ans = 0;
  string line;
  while (getline(cin, line)) {
    stringstream ss(line);
    vector<int> ns;
    int n;
    while (ss >> n)
      ns.push_back(n);
    if (backwards)
      reverse(ns.begin(), ns.end());
    ans += extrapolate(ns);
  }
  cout << ans << '\n';
}

void part1() { solve(false); }
void part2() { solve(true); }

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
