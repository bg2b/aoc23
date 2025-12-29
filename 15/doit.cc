// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cassert>

using namespace std;

vector<string> steps;

void read() {
  char c;
  steps.push_back("");
  while (cin >> c) {
    if (c == '\n')
      continue;
    else if (c == ',')
      steps.push_back("");
    else
      steps.back().push_back(c);
  }
}

uint8_t HASH(string const &s) {
  uint8_t current_value = 0;
  for (auto c : s) {
    current_value += uint8_t(c);
    current_value *= 17;
  }
  return current_value;
}

using lens = pair<string, int>;
using box = vector<lens>;

struct lenses {
  array<box, 256> boxes;

  void HASHMAP(string const &step);

  int focusing_power() const;
};

void lenses::HASHMAP(string const &step) {
  assert(step.length() >= 2);
  size_t op_pos = step.length() - (step.back() == '-' ? 1 : 2);
  char op = step[op_pos];
  assert(op == '-' || op == '=');
  string label(step.begin(), step.begin() + op_pos);
  assert(!label.empty());
  auto &box = boxes[HASH(label)];
  auto p = find_if(box.begin(), box.end(),
                   [&](lens const &l) { return l.first == label; });
  if (op == '=') {
    int focal_length = step.back() - '0';
    if (p != box.end())
      p->second = focal_length;
    else
      box.emplace_back(label, focal_length);
  } else if (p != box.end())
    box.erase(p);
}

int lenses::focusing_power() const {
  int result = 0;
  for (size_t box_num = 0; box_num < boxes.size(); ++box_num) {
    auto const &box = boxes[box_num];
    for (size_t slot_num = 0; slot_num < box.size(); ++slot_num)
      result += (box_num + 1) * (slot_num + 1) * box[slot_num].second;
  }
  return result;
}

void part1() {
  size_t ans = 0;
  for (auto const &s : steps)
    ans += HASH(s);
  cout << ans << '\n';
}

void part2() {
  lenses ls;
  for (auto const &s : steps)
    ls.HASHMAP(s);
  cout << ls.focusing_power() << '\n';
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
