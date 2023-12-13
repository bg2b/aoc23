// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

using namespace std;

struct terrain {
  vector<string> rows;

  // First line given, read rest from cin
  terrain(string line);

  // Is there a horizontal reflection at row r, possibly considering
  // smudging?
  bool reflection(unsigned r, bool smudged) const;
  // Find horizontal reflection; return number of rows, 0 if no reflection
  unsigned reflection(bool smudged) const;

  // Flip so I don't have to think
  void transpose();

  unsigned summarize(bool smudged);
};

terrain::terrain(string line) {
  do {
    rows.push_back(line);
    getline(cin, line);
  } while (!line.empty());
}

bool smudged_match(string const &s1, string const &s2) {
  assert(s1.length() == s2.length());
  int mismatches = 0;
  for (size_t i = 0; i < s1.length(); ++i)
    if (s1[i] != s2[i])
      ++mismatches;
  return mismatches == 1;
}

bool terrain::reflection(unsigned r, bool smudged) const {
  for (unsigned r1 = r, r2 = r; r1-- > 0 && r2 < rows.size(); ++r2) {
    if (rows[r1] == rows[r2])
      continue;
    if (!smudged || !smudged_match(rows[r1], rows[r2]))
      return false;
    smudged = false;
  }
  // Smudging was expected and wasn't found => false
  return !smudged;
}

unsigned terrain::reflection(bool smudged) const {
  for (unsigned r = 1; r < rows.size(); ++r)
    if (reflection(r, smudged))
      return r;
  return 0;
}

void terrain::transpose() {
  vector<string> new_rows(rows.front().length());
  for (auto const &row : rows) {
    assert(row.length() == new_rows.size());
    for (size_t i = 0; i < row.length(); ++i)
      new_rows[i].push_back(row[i]);
  }
  rows = new_rows;
}

unsigned terrain::summarize(bool smudged) {
  unsigned above = 100 * reflection(smudged);
  transpose();
  unsigned left = reflection(smudged);
  assert((above != 0) != (left != 0));
  return above + left;
}

void solve(bool smudged) {
  unsigned ans = 0;
  string line;
  while (getline(cin, line))
    ans += terrain(line).summarize(smudged);
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
