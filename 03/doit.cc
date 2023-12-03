// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <cctype>
#include <map>
#include <vector>
#include <optional>
#include <cassert>

using namespace std;

using coord = pair<int, int>;

struct schematic {
  // Numbers are stored at the coord of the first digit
  map<coord, int> nums;
  // Parts
  map<coord, char> symbol;

  schematic();

  char part(coord const &xy) const {
    auto p = symbol.find(xy);
    return p == symbol.end() ? '.' : p->second;
  }

  // What part (if any) is the number n at xy attached to?
  optional<coord> part(coord const &xy, int n) const;

  void total_part_nums() const;
  void total_gear_ratios() const;
};

schematic::schematic() {
  coord xy{0, 0};
  // The number that's being read, if any
  optional<coord> num_xy;
  string row;
  while (getline(cin, row)) {
    xy.first = 0;
    // Careful, consider two rows ...123 / 456...
    num_xy.reset();
    for (auto c : row) {
      if (isdigit(c)) {
        if (!num_xy) {
          // No leading zeros allowed, else have to save digit counts
          assert(c != '0');
          num_xy = xy;
        }
        nums[*num_xy] = 10 * nums[*num_xy] + c - '0';
      } else {
        num_xy.reset();
        if (c != '.')
          symbol[xy] = c;
      }
      ++xy.first;
    }
    ++xy.second;
  }
}

optional<coord> schematic::part(coord const &xy, int n) const {
  optional<coord> result;
  int num_digits = 1;
  while ((n /= 10) > 0)
    ++num_digits;
  //    v position x, num_digits = 9
  // ..@123456789@...
  //   ^ x-1     ^ x+num_digits
  for (int x = xy.first - 1; x <= xy.first + num_digits; ++x)
    for (int y = xy.second - 1; y <= xy.second + 1; ++y)
      if (part({x, y}) != '.') {
        // Numbers shouldn't be associated with more than one part
        assert(!result);
        result = {x, y};
      }
  return result;
}

void schematic::total_part_nums() const {
  int total = 0;
  for (auto const &[xy, n] : nums)
    if (part(xy, n))
      total += n;
  cout << total << '\n';
}

void schematic::total_gear_ratios() const {
  map<coord, vector<int>> possible_gears;
  for (auto const &[xy, n] : nums) {
    auto pxy = part(xy, n);
    if (pxy && part(*pxy) ==  '*')
      possible_gears[*pxy].push_back(n);
  }
  int total = 0;
  for (auto const &[_, pns] : possible_gears)
    if (pns.size() == 2)
      total += pns[0] * pns[1];
  cout << total << '\n';
}

void part1() { schematic().total_part_nums(); }
void part2() { schematic().total_gear_ratios(); }

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
