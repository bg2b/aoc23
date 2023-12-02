// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <array>

using namespace std;

struct game {
  int id;
  array<int, 3> d{0, 0, 0};

  game(string const &s);

  int part1() const { return d[0] <= 12 && d[1] <= 13 && d[2] <= 14 ? id : 0; }
  int power() const { return d[0] * d[1] * d[2]; }
};

game::game(string const &s) {
  stringstream ss(s);
  string Game, colon, color;
  int n;
  ss >> Game >> id >> colon;
  while (ss >> n >> color) {
    int index = color[0] == 'r' ? 0 : color[0] == 'g' ? 1 : 2;
    d[index] = max(d[index], n);
  }
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
