// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <map>
#include <functional>

using namespace std;

void solve(function<int(int cn, int won)> const &worth) {
  string line;
  int total = 0;
  while (getline(cin, line)) {
    stringstream ss(line);
    // It don't mean a thing, if it ain't got that string
    string thing;
    int cn;
    ss >> thing >> cn >> thing;
    set<int> potential;
    while (ss >> thing && thing != "|")
      potential.insert(stoi(thing));
    int won;
    for (won = 0; ss >> thing; won += potential.count(stoi(thing)))
      ;
    total += worth(cn, won);
  }
  cout << total << '\n';
}

void part1() {
  solve([](int, int won) { return won == 0 ? 0 : (1 << (won - 1)); });
}

void part2() {
  map<int, int> num_copies;
  solve([&](int cn, int won) {
          int num_cn = ++num_copies[cn];
          for (int won_card = cn + 1; won_card <= cn + won; ++won_card)
            num_copies[won_card] += num_cn;
          return num_cn;
        });
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
