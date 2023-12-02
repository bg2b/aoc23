// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <array>
#include <vector>
#include <cassert>

using namespace std;

using dice = array<int, 3>;

struct game {
  int id;
  vector<dice> rounds;

  game(string const &s);

  // What's required to play the game?
  dice required() const;

  int part1() const {
    auto d = required();
    return d[0] <= 12 && d[1] <= 13 && d[2] <= 14 ? id : 0;
  }

  int power() const { auto d = required(); return d[0] * d[1] * d[2]; }
};

game::game(string const &s) {
  stringstream ss(s);
  string Game, colon;
  ss >> Game >> id >> colon;
  for (bool round_expected = true; round_expected; ) {
    dice rgb{0, 0, 0};
    for (bool rgb_expected = true; rgb_expected; ) {
      int num;
      string color;
      ss >> num >> color;
      assert(ss && !color.empty());
      assert(color[0] == 'r' || color[0] == 'g' || color[0] == 'b');
      rgb[color[0] == 'r' ? 0 : color[0] == 'g' ? 1 : 2] = num;
      rgb_expected = color.back() == ',';
      round_expected = color.back() == ';';
    }
    rounds.push_back(rgb);
  }
}

dice game::required() const {
  dice rgb{0, 0, 0};
  for (auto const &round : rounds)
    for (int i = 0; i < 3; ++i)
      rgb[i] = max(rgb[i], round[i]);
  return rgb;
}

void sum(int (game::*value)() const) {
  string line;
  int ans = 0;
  while (getline(cin, line))
    ans += (game(line).*value)();
  cout << ans << '\n';
}

void part1() { sum(&game::part1); }
void part2() { sum(&game::power); }

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
