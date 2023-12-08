// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <map>
#include <numeric>
#include <cassert>

using namespace std;

string steps;
map<string, pair<string, string>> network;

void read() {
  cin >> steps;
  string start, equals, left, right;
  while (cin >> start >> equals >> left >> right) {
    assert(network.find(start) == network.end());
    assert(left.length() == 5 && right.length() == 4);
    network.emplace(start, make_pair(left.substr(1, 3), right.substr(0, 3)));
  }
}

int perchance_to_dream(string loc) {
  int num_steps = 0;
  while (loc.back() != 'Z') {
    assert(network.find(loc) != network.end());
    auto const &[left, right] = network.find(loc)->second;
    loc = steps[num_steps++ % steps.length()] == 'L' ? left : right;
  }
  // The code for part 2 relies on the assumption that the network
  // cycles with the same period and ends exactly at the end of the
  // number of steps.  The code below verifies this at the cost of
  // going around twice.
  /*
  assert(num_steps % steps.length() == 0);
  int num_steps1 = num_steps;
  string loc1 = loc;
  do {
    assert(network.find(loc) != network.end());
    auto const &[left, right] = network.find(loc)->second;
    loc = steps[num_steps1++ % steps.length()] == 'L' ? left : right;
  } while (loc.back() != 'Z');
  assert(loc1 == loc && num_steps1 == 2 * num_steps);
  */
  return num_steps;
}

void part1() { cout << perchance_to_dream("AAA") << '\n'; }

void part2() {
  long ans = 1;
  for (auto const &[start, _] : network)
    if (start.back() == 'A')
      ans = lcm(ans, perchance_to_dream(start));
  cout << ans << '\n';
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
