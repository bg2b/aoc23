// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit1 doit1.cc
// ./doit1 1 < input  # part 1
// ./doit1 2 < input  # part 2

// This is much faster than the more naive approach in doit.cc.  The
// idea is to consider the digraph whose nodes are pairs of a tile and
// an entering direction.  Connectivity in the digraph is what you'd
// naturally expect: (t1, dir1) has an edge to (t2, dir2) if shooting
// a beam into tile t1 with direction dir1 leads to a beam going into
// tile t2 with direction dir2.  So basically t2 is adjacent to t1,
// and dir2 just reflects ;-) whatever happens to dir1 in its
// interaction with t1.
//
// The digraph has a set of strongly-connected components (SCCs), that
// is, collections of nodes where every node in an SCC can reach every
// other node in the same SCC.  The SCCs form a tree; if nodes v1 and
// v2 are in distinct SCCs and v1 can reach v2, then v2 cannot reach
// v1 (otherwise they'd be in a single SCC).  Computation of the SCCs
// in a digraph can be done efficiently via various algorithms,
// including the one due to Tarjan:
// https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm
//
// The inputs for this problem are such that there are a few big SCCs,
// but most SCCs are just a single tile and direction.  Dealing with
// the latter is pointless (it's basically just the beam traversing
// the tile in the usual way).  However, consider what happens when
// the beam hits some (t, dir) that is in a nontrivial SCC.  By
// definition, there will be loops as the beam bounces around, with
// eventually some beams returning to (t, dir).  And some beams will
// leave the SCC, going to other SCCs and (eventually) out of the
// cave.  Whatever happens, *the exact same thing* would happen for
// every (t', dir') that's in the same SCC as (t, dir).  This suggests
// that every big SCC should just be analyzed once: find the big SCCs,
// do beam tracing for each one, and just record that beam info.
//
// Consider now shooting a beam into the cave from some edge.  The
// (edge tile, incoming direction) pair is a trivial SCC (a beam could
// get back to the same edge tile, but nothing could reflect it back
// into the cave again).  As long as the beam is going through trivial
// SCCs, just beam trace as normal.  But if the beam hits a nontrivial
// SCC, cut off the search immediately, just remembering which SCC was
// reached.  The beam from the edge may reached several nontrivial
// SCCs; all of them have to be noted.  Once the search terminates,
// there are two pieces of information:
// 1. The (t, dir) that were explicitly traversed
// 2. The set of nontrivial SCCs that were reached
// If the set 2 is empty, just count energized tiles in 1 as usual.
// Otherwise, scan the whole cave.  A tile t is energized if it was
// traversed explicitly, or if there is some dir where (t, dir) is
// part of one of the nontrivial SCCs that was reached.
//
// There might be some way to actually not have to scan the whole cave
// when counting energized tiles.  Couldn't the tiles energized by an
// SCC just be counted once?  The problem is that energized is a
// property of a tile, while an SCC involves a direction too.  So a
// single tile may be part of two completely disjoint SCCs (e.g.,
// consider an empty tile where an up-down beam in one SCC and an
// left-right beam in another SCC cross).  It gets complicated, and
// tiles that were also energized during the explicit trace might also
// have been energized by SCCs.  Overall it just seems easiest to scan
// everything, where it's possible to see all the different ways that
// a given tile was energized and to count the tile exactly once.

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <stdexcept>
#include <cassert>

using namespace std;

// Mathematical conventions are used: x coordinate ([0]) is
// horizontal, y coordinate positive is up on the screen, so lower
// left corner on the picture of the layout is (0, 0)

using coord = array<int, 2>;

coord operator+(coord const &c1, coord const &c2) {
  return {c1[0] + c2[0], c1[1] + c2[1]};
}

vector<coord> directions;

int find_dir(coord const &c) {
  for (size_t dir = 0; dir < directions.size(); ++dir)
    if (c == directions[dir])
      return dir;
  directions.push_back(c);
  return directions.size() - 1;
}

int const rt = find_dir({+1, 0});
int const up = find_dir({0, +1});
int const lt = find_dir({-1, 0});
int const dn = find_dir({0, -1});

// State for strongly-connected component analysis (Tarjan)
// index, lowlink, on_stack
using scc_state = tuple<int, int, bool>;

// Initial state of a node (-1 means unscanned so far)
scc_state const undefined = {-1, 0, false};

// Bit set of (indexes of) SCCs.  Something else that accommodates
// arbitrary numbers of SCCs could be used, but in the actual inputs
// it seems like there are at most ~10 nontrivial SCCs.
using scc_set = uint64_t;

struct tile {
  // What's in the tile?
  char contents;
  // Which directions have beams been (explicitly) shot into the tile?
  char beam_dirs{0};
  // State for SCC analysis (the tile is effectively 4 nodes, one for
  // each direction).  During initial SCC computation, index -1 is for
  // undefined.  After the analysis, index is either the SCC number
  // (0, 1, ...) if the tile/direction is part of a nontrivial SCC, or
  // -1 for a trivial SCC of size 1.
  array<scc_state, 4> scc{undefined, undefined, undefined, undefined};
  // scc_beam_dirs[i] records the state of beam_dirs after shooting a
  // beam through SCC i.  Note that scc_beam_dirs[i] being nonzero
  // does not mean that this tile is part of SCC i.  It could also
  // have been reached downstream from that SCC.
  vector<char> scc_beam_dirs;

  tile(char c) : contents(c) {}

  // Clear directly-trace state
  void reset() { beam_dirs = 0; }
  // Considering the SCCs hit and the directly-traced state, was the
  // tile energized?  This also resets beam_dirs.
  bool energized(scc_set sccs = 0);

  // Return the outgoing directions for a beam that is coming in from
  // direction dir.  If the incoming beam has already been traced (as
  // recorded by beam_dirs), returns an empty vector.
  vector<int> shoot(int dir);

  // Is the given direction part of a nontrivial SCC?  If so, add the
  // SCC to the set and return true; otherwise return false.
  bool in_nontrivial_scc(int dir, scc_set &sccs) const;

  // Save beam_dirs from a strongly-connected component analysis.
  // (Also resets beam_dirs.)
  void record_scc() { scc_beam_dirs.push_back(beam_dirs); reset(); }

  // For my own sanity...
  static void verify();
};

bool tile::energized(scc_set sccs) {
  bool result = (beam_dirs != 0);
  reset();
  for (int i = 0; sccs != 0; ++i, sccs >>= 1)
    if ((sccs & 1) != 0 && scc_beam_dirs[i] != 0)
      result = true;
  return result;
}

vector<int> tile::shoot(int dir) {
  if ((beam_dirs & (1 << dir)) != 0)
    // Already explicitly traced
    return {};
  beam_dirs |= 1 << dir;
  if (contents == '/')
    return {find_dir({directions[dir][1], directions[dir][0]})};
  if (contents == '\\')
    return {find_dir({-directions[dir][1], -directions[dir][0]})};
  bool horizontal = directions[dir][0] != 0;
  if (contents == '|' && horizontal)
    return {up, dn};
  if (contents == '-' && !horizontal)
    return {rt, lt};
  return {dir};
}

bool tile::in_nontrivial_scc(int dir, scc_set &sccs) const {
  // This is for after SCC analysis, when the first component has been
  // set to either the SCC number (0, 1, ...) or to -1 meaning just a
  // trivial size SCC of size 1
  int index = get<0>(scc[dir]);
  if (index == -1)
    return false;
  sccs |= scc_set(1) << index;
  return true;
}

void tile::verify() {
  assert(tile('/').shoot(rt) == vector<int>{up});
  assert(tile('/').shoot(lt) == vector<int>{dn});
  assert(tile('/').shoot(up) == vector<int>{rt});
  assert(tile('/').shoot(dn) == vector<int>{lt});
  assert(tile('\\').shoot(rt) == vector<int>{dn});
  assert(tile('\\').shoot(lt) == vector<int>{up});
  assert(tile('\\').shoot(up) == vector<int>{lt});
  assert(tile('\\').shoot(dn) == vector<int>{rt});
  vector<int> updown{up, dn};
  assert(tile('|').shoot(rt) == updown);
  assert(tile('|').shoot(lt) == updown);
  vector<int> rightleft{rt, lt};
  assert(tile('-').shoot(up) == rightleft);
  assert(tile('-').shoot(dn) == rightleft);
  assert(tile('.').shoot(up) == vector<int>{up});
  assert(tile('|').shoot(dn) == vector<int>{dn});
  assert(tile('-').shoot(rt) == vector<int>{rt});
  tile t('.');
  assert(t.shoot(rt) == vector<int>{rt});
  assert(t.shoot(rt).empty());
  assert(t.energized());
}

// SCCs are analyzed in the digraph whose nodes are (tile, direction)
using node = pair<coord, int>;

struct cave {
  // The tiles in read order (i.e., layout[0] is the top row, which is
  // at y == height - 1)
  vector<vector<tile>> layout;

  cave();

  int width() const { return layout.front().size(); }
  int height() const { return layout.size(); }
  bool in_bounds(coord const &c) const;
  tile &at(coord const &c);

  // Utility for SCC analysis.  What are the successor nodes of the
  // given node?
  vector<node> successors(node const &v);

  // Do SCC analysis and beam trace in the nontrivial SCCs
  void tarjan();

  // The shoot() method is used for three things:
  // 1. Direct beam tracing, ignoring any SCC information
  // 2. Tracing while using SCC information
  // 3. Tracing an SCC and saving the resulting search state
  // This is used to specify what should be done
  enum scc_action { ignore_sccs = 0, consider_sccs, record_sccs };
  // Shoot a light ray in at start in direction start_dir, possibly
  // considering SCCs.  If the action is record_sccs, just save the
  // beam state afterwards for the SCC that was shot (and return 0).
  // Otherwise return the number of energized tiles.
  int shoot(coord const &start, int start_dir, scc_action action);
};

cave::cave() {
  string line;
  while (cin >> line) {
    layout.push_back(vector<tile>());
    for (auto c : line)
      layout.back().emplace_back(tile(c));
    assert(layout.back().size() == layout.front().size());
  }
}

bool cave::in_bounds(coord const &c) const {
  return c[0] >= 0 && c[0] < width() && c[1] >= 0 && c[1] < height();
}

tile &cave::at(coord const &c) {
  return layout[height() - 1 - c[1]][c[0]];
}

vector<node> cave::successors(node const &v) {
  auto [c, dir] = v;
  vector<node> result;
  for (int next_dir : at(c).shoot(dir)) {
    auto next_c = c + directions[next_dir];
    if (in_bounds(next_c))
      result.emplace_back(next_c, next_dir);
  }
  // Be sure to clear the beam state, since I'm not really shooting
  // beams and I don't want any history.
  at(c).reset();
  return result;
}

void cave::tarjan() {
  int next_index = 0;
  vector<vector<node>> nontrivial_sccs;
  vector<node> stack;
  // Auxiliary function returning a node's associated info
  auto info = [&](node const &v) -> scc_state & {
                auto [c, dir] = v;
                return at(c).scc[dir];
              };
  // Basically the pseudo-code from the Wikipedia article cited in the
  // explanation at the start
  auto connect =
    [&](node const &v, auto &connect_) -> void {
      auto &[v_index, v_lowlink, v_on_stack] = info(v);
      assert(v_index == -1);
      v_index = next_index++;
      v_lowlink = v_index;
      stack.push_back(v);
      v_on_stack = true;
      for (auto w : successors(v)) {
        auto &[w_index, w_lowlink, w_on_stack] = info(w);
        if (w_index == -1) {
          connect_(w, connect_);
          v_lowlink = min(v_lowlink, w_lowlink);
        } else if (w_on_stack)
          v_lowlink = min(v_lowlink, w_index);
      }
      if (v_lowlink == v_index) {
        vector<node> scc;
        while (true) {
          auto w = stack.back();
          stack.pop_back();
          get<2>(info(w)) = false;
          scc.push_back(w);
          if (w == v)
            break;
        }
        if (scc.size() > 1)
          nontrivial_sccs.emplace_back(move(scc));
      }
    };
  // Scan all nodes to find SCCs
  for (int x = 0; x < width(); ++x)
    for (int y = 0; y < height(); ++y)
      for (int dir = 0; dir < 4; ++dir) {
        node v{coord{x, y}, dir};
        if (get<0>(info(v)) == -1)
          connect(v, connect);
      }
  // Mark all nodes as being not part of an SCC
  for (auto &row : layout)
    for (auto &t : row)
      for (int dir = 0; dir < 4; ++dir)
        get<0>(t.scc[dir]) = -1;
  // Fill in the nontrivial SCCs, shoot one state from each SCC (which
  // will mark the SCC plus other stuff reachable from it), and
  // collect the beam states
  unsigned scc_index = 0;
  for (auto const &scc : nontrivial_sccs) {
    for (auto v : scc)
      get<0>(info(v)) = scc_index;
    auto [c, dir] = scc.front();
    shoot(c, dir, record_sccs);
    ++scc_index;
  }
  // Make sure I didn't overflow the scc_set range
  assert(scc_index < 8 * sizeof(scc_set));
}

int cave::shoot(coord const &start, int start_dir, scc_action action) {
  vector<pair<coord, int>> to_shoot;
  to_shoot.emplace_back(start, start_dir);
  scc_set sccs = 0;
  size_t next = 0;
  while (next < to_shoot.size()) {
    auto [c, dir] = to_shoot[next];
    ++next;
    auto &t = at(c);
    if (action == consider_sccs && t.in_nontrivial_scc(dir, sccs))
      // Cutoff due to reaching a nontrivial SCC
      continue;
    for (int next_dir : t.shoot(dir)) {
      auto nc = c + directions[next_dir];
      if (in_bounds(nc))
        to_shoot.emplace_back(nc, next_dir);
    }
  }
  int result = 0;
  if (action == record_sccs)
    // (start, start_dir) was for a nontrivial SCC; save the beam
    // tracing for the SCC
    for (auto &row : layout)
      for (auto &t : row)
        t.record_scc();
  else if (sccs == 0) {
    // No SCCs hit (or not using SCCs); everything energized is in
    // to_shoot.  Note that the same tile may be there more than once,
    // so can't just use to_shoot.size().  And I need to reset the
    // tiles anyway.
    for (auto [c, _] : to_shoot)
      if (at(c).energized())
        ++result;
  } else
    // Have to look at everything to include the SCC contributions
    for (auto &row : layout)
      for (auto &t : row)
        if (t.energized(sccs))
          ++result;
  return result;
}

void part1() {
  cave cv;
  cout << cv.shoot(coord{0, cv.height() - 1}, rt, cave::ignore_sccs) << '\n';
}

void part2() {
  cave cv;
  cv.tarjan();
  int ans = 0;
  for (int x = 0; x < cv.width(); ++x) {
    ans = max(ans, cv.shoot({x, 0}, up, cave::consider_sccs));
    ans = max(ans, cv.shoot({x, cv.height() - 1}, dn, cave::consider_sccs));
  }
  for (int y = 0; y < cv.height(); ++y) {
    ans = max(ans, cv.shoot({0, y}, rt, cave::consider_sccs));
    ans = max(ans, cv.shoot({cv.width() - 1, y}, lt, cave::consider_sccs));
  }
  cout << ans << '\n';
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " partnum < input\n";
    exit(1);
  }
  tile::verify();
  if (*argv[1] == '1')
    part1();
  else
    part2();
  return 0;
}
