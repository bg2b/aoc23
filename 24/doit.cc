// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <vector>
#include <array>
#include <numeric>
#include <optional>
#include <cmath>
#include <cassert>

using namespace std;

using vec = array<long, 3>;
using hailstone = pair<vec, vec>;

vec operator+(vec const &v1, vec const &v2) {
  return {v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]};
}

vec operator-(vec const &v1, vec const &v2) {
  return {v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]};
}

vec operator*(long sc, vec const &v) {
  return {sc * v[0], sc * v[1], sc * v[2]};
}

vec operator/(vec const &v, long sc) {
  assert(v[0] % sc == 0 && v[1] % sc == 0 && v[2] % sc == 0);
  return {v[0] / sc, v[1] / sc, v[2] / sc};
}

int nonzero_index(vec const &v) {
  assert(v != vec({0, 0, 0}));
  return v[0] != 0 ? 0 : (v[1] != 0 ? 1 : 2);
}

// Scale out the gcd of the components; used when only interested in
// the direction of a vector
vec direction(vec const &v) {
  long d = gcd(gcd(v[0], v[1]), v[2]);
  if (d == 0)
    return v;
  return {v[0] / d, v[1] / d, v[2] / d};
}

vector<hailstone> read() {
  vector<hailstone> result;
  auto read_vec = [](vec &v) {
                    char comma;
                    return bool(cin >> v[0] >> comma >> v[1] >> comma >> v[2]);
                  };
  vec pos;
  vec vel;
  string at;
  while (read_vec(pos) && (cin >> at) && read_vec(vel))
    result.emplace_back(pos, vel);
  return result;
}

// Solve [a b] == [e]
//       [c d]    [f]
pair<double, double> solve2x2(double a, double b, double c, double d,
                              double e, double f) {
  double det = a * d - b * c;
  if (det == 0.0)
    return {nan(""), nan("")};
  double x1 = +d * e + -b * f;
  double x2 = -c * e + +a * f;
  return {x1 / det, x2 / det};
}

// Check xy intersection times for two hailstones, see if both are in
// the future and if the intersection occurs within the box of
// interest
bool xy_intersect_in_box(hailstone const &h1, hailstone const &h2) {
  // x1 + vx1*t1 = x2 + vx2*t2
  // y1 + vy1*t1 = y2 + vy2*t2
  //
  // vx1*t1 - vx2*t2 = x2 - x1
  // vy1*t1 - vy2*t2 = y2 - y1
  //
  // [vx1 -vx2] * [t1] = [x2-x1]
  // [vy1 -vy2]   [t2]   [y2-y1]
  auto const &[p1, v1] = h1;
  auto const &[p2, v2] = h2;
  auto [x1, y1, z1] = p1;
  auto [vx1, vy1, vz1] = v1;
  auto [x2, y2, z2] = p2;
  auto [vx2, vy2, vz2] = v2;
  auto [t1, t2] = solve2x2(vx1, -vx2, vy1, -vy2, x2 - x1, y2 - y1);
  if (isnan(t1))
    // No intersection
    return false;
  if (t1 < 0.0 || t2 < 0.0)
    // In the past for someone
    return false;
  // The intersection point
  double x1t1 = x1 + vx1 * t1;
  double y1t1 = y1 + vy1 * t1;
  double x2t2 = x2 + vx2 * t2;
  double y2t2 = y2 + vy2 * t2;
  auto close = [](double a, double b) {
                 return fabs(a - b) < 1e-10 * (fabs(a) + fabs(b));
               };
  assert(close(x1t1, x2t2) && close(y1t1, y2t2));
  double const ll = 200000000000000.0;
  double const ur = 400000000000000.0;
  return ll <= x1t1 && x1t1 <= ur && ll <= y1t1 && y1t1 <= ur;
}

void part1() {
  int ans = 0;
  auto stones = read();
  for (size_t i = 0; i < stones.size(); ++i)
    for (size_t j = i + 1; j < stones.size(); ++j)
      if (xy_intersect_in_box(stones[i], stones[j]))
        ++ans;
  cout << ans << '\n';
}

// Cross product, but the input vectors aren't normalized (they have
// integer coordinates), and the output scaling isn't determined
// either.  That is, I want a vector of integer coordinates (having
// gcd 1) pointing in the direction of the cross product.
vec directional_cross(vec a, vec b) {
  a = direction(a);
  b = direction(b);
  vec c;
  c[0] = a[1] * b[2] - a[2] * b[1];
  c[1] = a[2] * b[0] - a[0] * b[2];
  c[2] = a[0] * b[1] - a[1] * b[0];
  return direction(c);
}

long dot(vec const &a, vec const &b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// Find how to throw a rock (position and velocity) to hit three
// stones.  The idea is to shift into the reference frame of one of
// the hailstones so that it's just sitting still at the origin.  The
// other two stones are still flying along lines in this frame.  A
// throw that hits the origin and also one of those stones must lie in
// the plane defined by the origin and the line of the stone.  And the
// same for the other moving stone.  So there are two planes, and the
// velocity vector of interest must be along the intersection of those
// two planes.
//
// Given the the direction of the velocity vector, then in the shifted
// frame I should see the two moving stones crossing through the
// origin when projecting in that direction.  Importantly, the times
// at which they do that are *not* affected by the shifting and
// projection.  So I just compute those crossing times, and (assuming
// that the times are not the same) everything becomes trivial.  Just
// take the position difference of those two stones at those times (in
// the original unshifted frame), scale by the time difference to get
// the throw velocity, and extrapolate to time 0 for the throw
// position.
//
// There are various possible degenerate cases, e.g., the reference
// stone and one of the other stones moving with the same velocity, so
// the second stone doesn't define a unique plane.  Probably the input
// is constructed such that these never happen, but the routine should
// return both throw position and velocity {0, 0, 0} in such an event.
pair<vec, vec> thread_the_needle(hailstone const &h1,
                                 hailstone const &h2,
                                 hailstone const &h3) {
  auto [p1, v1] = h1;
  auto [p2, v2] = h2;
  auto [p3, v3] = h3;
  // Shift to frame of h1
  p2 = p2 - p1;
  v2 = v2 - v1;
  p3 = p3 - p1;
  v3 = v3 - v1;
  // Normals to planes
  vec n2 = directional_cross(p2, p2 + v2);
  vec n3 = directional_cross(p3, p3 + v3);
  // Line of velocity vector
  vec v = directional_cross(n2, n3);
  if (v == vec{0, 0, 0})
    // Some sort of degeneracy
    return {v, v};
  // Utility for getting the hit time of one of h2 or h3 by looking at
  // the time for the projected stone to get to the origin.  If v were
  // normalized, then the projection would just be hp - (v'*hp)*v for
  // the stone's position and hv - (v'*hv)*v for the stone's velocity.
  // Because v isn't normalized and I want to keep everything as
  // integers, I'll scale the whole equation by v'*v.
  auto vv = dot(v, v);
  auto hit_time = [&](vec hp, vec hv) {
                    // Project
                    hp = vv * hp - dot(v, hp) * v;
                    hv = vv * hv - dot(v, hv) * v;
                    // It's a linear equation; just take one nonzero
                    // coordinate and solve
                    int x = nonzero_index(hv);
                    assert(hp[x] % hv[x] == 0);
                    return -hp[x] / hv[x];
                  };
  auto t2 = hit_time(p2, v2);
  auto t3 = hit_time(p3, v3);
  if (t2 == t3)
    // This likely won't happen with the real input, but one could
    // certainly make an example where it would
    return {vec{0, 0, 0}, vec{0, 0, 0}};
  assert(t2 >= 0 && t3 >= 0 && t2 != t3);
  // Once I know the times, it's easy...  Where are h2 and h3 at those
  // times in the original frame?
  auto hit_pos2 = h2.first + t2 * h2.second;
  auto hit_pos3 = h3.first + t3 * h3.second;
  // The actual velocity vector for the rock follows from those and
  // the time difference
  v = (hit_pos3 - hit_pos2) / (t3 - t2);
  // Now just extrapolate to where the rock must have been at time 0
  return {hit_pos2 - t2 * v, v};
}

void part2() {
  auto stones = read();
  assert(stones.size() >= 3);
  auto [p, v] = thread_the_needle(stones[0], stones[1], stones[2]);
  // If this ever failed because of a degeneracy, I'd need to pick
  // three other stones
  assert(v != vec({0, 0, 0}));
  auto is_hit = [&](hailstone const &h) {
                  auto [hp, hv] = h;
                  // hp + t*hv = p + t*v
                  auto dp = p - hp;
                  auto dv = hv - v;
                  int x = nonzero_index(dv);
                  auto t = dp[x] / dv[x];
                  return hp + t * hv == p + t * v;
                };
  // Just to verify...
  for (auto const &h : stones)
    assert(is_hit(h));
  cout << p[0] + p[1] + p[2] << '\n';
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
