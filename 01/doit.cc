// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <cassert>
#include <string>
#include <map>
#include <optional>

using namespace std;

// For part 2, can't just blindly replace spelled-out versions with
// single digits and then apply a simple part 1.  Consider "eighthree"
// or "oneight".  There's at least one instance in my input with
// "eightwo" at the beginning, where it would matter which way it's
// interpreted.

int calibration(string const &s, map<string, int> const &digits) {
  optional<pair<size_t, int>> fdigit, ldigit;
  for (auto const &[digit, val] : digits) {
    auto p = make_pair(s.find(digit), val);
    if (p.first != string::npos && p.first <= fdigit.value_or(p).first)
      fdigit = p;
    p.first = s.rfind(digit);
    if (p.first != string::npos && p.first >= ldigit.value_or(p).first)
      ldigit = p;
  }
  assert(fdigit.has_value() && ldigit.has_value());
  return 10 * fdigit->second + ldigit->second;
}

void solve(map<string, int> const &digits) {
  int sum = 0;
  string line;
  while (getline(cin, line))
    sum += calibration(line, digits);
  cout << sum << '\n';
}

map<string, int> basic_digits = {
    {"0", 0}, {"1", 1}, {"2", 2}, {"3", 3}, {"4", 4},
    {"5", 5}, {"6", 6}, {"7", 7}, {"8", 8}, {"9", 9},
};

void part1() { solve(basic_digits); }

void part2() {
  auto digits = basic_digits;
  int val = 1;
  for (auto digit :
       {"one", "two", "three", "four", "five", "six", "seven", "eight", "nine"})
    digits[digit] = val++;
  solve(digits);
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
