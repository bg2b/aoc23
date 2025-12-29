// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <cassert>

using namespace std;

bool using_joker = false;
string card_order = "23456789TJQKA";

void jokers_wild() {
  // Am I the only one that remembers that show?
  using_joker = true;
  card_order = "J23456789TQKA";
}

struct hand {
  string cards;
  int bid;
  enum { high = 0, one_pair, two_pair, three, full_house, four, five } type;

  hand(string const &s);
};

hand::hand(string const &s) : cards(s.substr(0, 5)), bid(stoi(s.substr(6))) {
  map<char, int> counts;
  for (auto c : cards)
    ++counts[c];
  // NB: not && counts['J'] > 0, which implicitly does counts['J'] = 0
  // if 'J' isn't there.  Don't ask me how I know...
  if (using_joker && cards.find('J') != string::npos) {
    // The Count counts everything; how could this not be true?
    assert(counts.count('J'));
    // When using the joker, it's always at least as good for the hand
    // type to choose them all to be the same.  This is obvious with
    // three or four jokers.  With two, the question would be whether
    // splitting the types would let us make a full house?  If it
    // could, then we'd have had XYYJJ, with one J being X and one
    // being Y, but that would have been better as 4-of-a-kind with
    // both J's being Y.
    //
    // So what should the jokers be?  Claim: it's best to increase the
    // count of the most common card.  Going one_pair => three => four
    // => five is obvious.  If there's one J and two_pair, it doesn't
    // matter which count we increase; it'll become a full_house.  And
    // with one J and nothing else, it'll become one_pair no matter
    // which which card is bumped.
    int num_jokers = counts['J'];
    if (num_jokers < 5) {
      // There's something else whose count can be increased
      counts.erase('J');
      char most_common = counts.begin()->first;
      for (auto [c, count] : counts)
        if (counts[c] > counts[most_common])
          most_common = c;
      counts[most_common] += num_jokers;
    }
  }
  type = high;
  for (auto [_, count] : counts)
    switch (count) {
    case 2:
      type = max(type, one_pair);
      break;
    case 3:
      type = max(type, three);
      break;
    case 4:
      type = max(type, four);
      break;
    case 5:
      type = max(type, five);
      break;
    }
  // Full house?
  if (type == three && counts.size() == 2)
    type = full_house;
  // Two pair?
  if (type == one_pair && counts.size() == 3)
    type = two_pair;
}

bool operator<(hand const &h1, hand const &h2) {
  assert(h1.cards != h2.cards);
  if (h1.type != h2.type)
    return h1.type < h2.type;
  for (int i = 0; i < 5; ++i)
    if (h1.cards[i] != h2.cards[i])
      return card_order.find(h1.cards[i]) < card_order.find(h2.cards[i]);
  throw logic_error("This cannot happen");
}

void solve() {
  vector<hand> hands;
  string line;
  while (getline(cin, line))
    hands.emplace_back(line);
  sort(hands.begin(), hands.end());
  int ans = 0;
  for (size_t i = 0; i < hands.size(); ++i)
    ans += (i + 1) * hands[i].bid;
  cout << ans << '\n';
}

void part1() { solve(); }
void part2() {
  jokers_wild();
  solve();
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
