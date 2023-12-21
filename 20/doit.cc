// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <cassert>

using namespace std;

// From, to, msg
using id = unsigned;
using pulse = tuple<id, id, char>;

struct modyule;
struct watched;

struct network {
  // Name table
  vector<string> names;
  // The elements
  vector<unique_ptr<modyule>> mods;
  // Everything that's been sent
  vector<pulse> pulses;
  // The broadcaster
  id bcast{0};

  // Get the id for a name, allocating a new one if needed
  id name_to_id(string const &name);

  // Read from cin
  network();

  // Send a pulse
  void send(id from, id to, char msg) { pulses.emplace_back(from, to, msg); }

  // Press the button
  void press();

  // For part 1
  pair<size_t, size_t> pulse_counts() const;
  // Figure out the answer for part 2
  size_t part2();
};

struct modyule {
  // Where am I?
  network &net;
  // WHO am I?
  id me;
  // Who do I send to?
  vector<id> output;
  // Who sends to me?
  vector<id> input;

  modyule(network &net_, id me_) : net(net_), me(me_) {}
  virtual ~modyule() {}

  // Add someone as one of my outputs
  void add_output(id out) { output.push_back(out); }
  // Add someone as one of my inputs
  virtual void link_to(id in) { input.push_back(in); }

  // I've got mail!
  virtual void receive(id from, char msg) = 0;
  // Send something to everyone on my xmas card list
  void send(vector<id> const &output, char msg) {
    for (id out : output)
      net.send(me, out, msg);
  }
};

struct flip_flop: public modyule {
  bool is_on{false};

  flip_flop(network &net_, id me_) : modyule(net_, me_) {}

  void receive(id, char msg);
};

void flip_flop::receive(id, char msg) {
  if (msg != '0')
    // Spam filtering...
    return;
  is_on = !is_on;
  send(output, is_on ? '1' : '0');
}

struct conjunct: public modyule {
  // What did I last receive from each input?
  vector<char> memory;
  // Did I ever output a '1'?
  bool fired{false};

  conjunct(network &net_, id me_) : modyule(net_, me_) {}

  void link_to(id in) {
    modyule::link_to(in);
    memory = vector<char>(input.size(), '0');
  }

  char &memory_of(id in);
  void receive(id from, char msg);
};

char &conjunct::memory_of(id in) {
  for (size_t i = 0; i < input.size(); ++i)
    if (input[i] == in)
      return memory[i];
  throw logic_error("This cannot happen");
}

void conjunct::receive(id from, char msg) {
  memory_of(from) = msg;
  for (char prev : memory)
    if (prev != '1') {
      send(output, '1');
      fired = true;
      return;
    }
  send(output, '0');
}

struct broadcaster: public modyule {
  broadcaster(network &net_, id me_) : modyule(net_, me_) {}

  void receive(id, char msg) { send(output, msg); }
};

// A dummy module to soak up whatever isn't otherwise consumed
struct sink: public modyule {
  sink(network &net_, id me_) : modyule(net_, me_) {}

  void receive(id, char) {}
};

network::network() {
  string line;
  while (getline(cin, line)) {
    stringstream ss(line);
    char type;
    string name;
    string to;
    ss >> type >> name >> to;
    assert(ss);
    assert(type == '%' || type == '&' || type == 'b');
    assert(to == "->");
    id i = name_to_id(name);
    if (type == '%')
      mods[i].reset(new flip_flop(*this, i));
    else if (type == '&')
      mods[i].reset(new conjunct(*this, i));
    else {
      // This sounds like the name of brand of guitar?
      assert(name == "roadcaster");
      mods[i].reset(new broadcaster(*this, i));
      bcast = i;
    }
    modyule &mod = *mods[i];
    while (ss >> to) {
      if (to.back() == ',')
        to.pop_back();
      assert(!to.empty());
      mod.add_output(name_to_id(to));
    }
  }
  for (id i = 0; i < mods.size(); ++i)
    if (!mods[i])
      mods[i].reset(new sink(*this, i));
  for (auto &mod : mods)
    for (auto out : mod->output)
      mods[out]->link_to(mod->me);
  assert(names[bcast] == "roadcaster");
}

id network::name_to_id(string const &name) {
  for (id i = 0; i < names.size(); ++i)
    if (names[i] == name)
      return i;
  names.push_back(name);
  mods.emplace_back(nullptr);
  return names.size() - 1;
}

void network::press() {
  pulses.clear();
  id const button = 19650127;
  // Get things going
  send(button, bcast, '0');
  // Propogate pulses
  for (size_t i = 0; i < pulses.size(); ++i) {
    auto [from, to, msg] = pulses[i];
    mods[to]->receive(from, msg);
  }
}

pair<size_t, size_t> network::pulse_counts() const {
  size_t low = 0;
  for (auto const &p : pulses)
    if (get<2>(p) == '0')
      ++low;
  return {low, pulses.size() - low};
}

size_t network::part2() {
  id rx = name_to_id("rx");
  assert(mods[rx] && mods[rx]->input.size() == 1);
  auto conj = [&](id cid) {
                return dynamic_cast<conjunct const *>(mods[cid].get());
              };
  // The conjunction feeding rx
  auto main_conj = conj(mods[rx]->input.front());
  // The conjunction inputs to that conjunction and how many times the
  // button was pressed to get them to fire.
  vector<pair<conjunct const *, size_t>> parts;
  for (auto cid : main_conj->input)
    parts.emplace_back(conj(cid), 1);
  bool all_fired;
  do {
    press();
    all_fired = true;
    for (auto &[conj, presses_to_fire] : parts)
      if (!conj->fired) {
        // Didn't fire after presses_to_fire, another one needed
        ++presses_to_fire;
        all_fired = false;
      }
  } while (!all_fired);
  size_t result = 1;
  for (auto const &[_, presses] : parts)
    result = lcm(result, presses);
  return result;
}

void part1() {
  network ntwk;
  size_t low = 0;
  size_t high = 0;
  for (int _ = 0; _ < 1000; ++_) {
    ntwk.press();
    auto [this_low, this_high] = ntwk.pulse_counts();
    low += this_low;
    high += this_high;
  }
  cout << low * high << '\n';
}

void part2() { cout << network().part2() << '\n'; }

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
