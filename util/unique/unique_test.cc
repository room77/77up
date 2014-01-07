// Copyright 2012 Room77, Inc.
// Author: Uygar Oztekin

#include <iostream>
#include <sstream>
#include <string>
#include "unique.h"

// Let's not depend on R77 test infrastructure for open source code.
// Define a basic ASSERT macro using assert if not present.
// Needs to be compiled in debug mode.
#ifndef ASSERT
#include <cassert>
#define ASSERT assert
#endif

using namespace std;

namespace {

void Sanity() {
  UNIQUE8(int) t1;
  UNIQUE8(int) t2;
  for(int i = 0; i < 256; ++i) {
    t1 = i;
    ASSERT(i == t1);
  }
  t2 = 256;
  ASSERT(sizeof(t1) == 1);
}

void WrapClassType() {
  // Unfortunately in order to access wrapped class' methods, we need to use
  // either .get() or operator().
  // I wish it was possible to overload operator. in C++!

  util::unique<string> s1;
  s1 = "Hello";
  s1 += " World";
  ASSERT(s1.get() == "Hello World");

  util::unique<string> s2 = s1 + "!";
  ASSERT(s2.get() == "Hello World!");
}

void WrapNonClassType() {
  util::unique<int> i1 = 10;
  i1 /= 2;
  ASSERT(i1 == 5);
  i1 += 2;
  ASSERT(i1 == 7);

  util::unique<int> i2 = i1 * 2;
  ASSERT(i2 == 14);
  int i3 = i2 / 7 + 1;
  ASSERT(i3 == 3);
  ASSERT(i1 + i2 + i3 == 24);
}

void ParseStream() {
  util::unique<int> i1 = 10;
  util::unique<int> i2 = 20;
  stringstream ss;
  ss << i2;
  ss >> i1;
  ASSERT(i1 == 20);
  ASSERT(i1 == i2);
  ASSERT(i2 == i1);
}

void Find() {
  util::unique<int> i1 = 10;
  util::unique<int> i2 = 20;
  util::unique<int> i3 = 40;

  pair<uint32_t, bool> found = util::unique<int>::storage_type::find(10);
  ASSERT(found.second);
  ASSERT(found.first == i1.key());

  found = util::unique<int>::storage_type::find(20);
  ASSERT(found.second);
  ASSERT(found.first == i2.key());

  found = util::unique<int>::storage_type::find(40);
  ASSERT(found.second);
  ASSERT(found.first == i3.key());

  found = util::unique<int>::storage_type::find(100);
  ASSERT(!found.second);
}

#ifdef R77_USE_SERIALIZER

struct tTest {
  string s;
  util::unique<string> us;
  int i;
  util::unique<int> ui;
  SERIALIZE(s*1 / us*2 / i*3 / ui*4);
  // CSV(s | us | i | ui);

  bool operator == (const tTest& r) const {
    return s == r.s && us == r.us && i == r.i && ui == r.ui;
  }
};

void BinarySerialization() {
  { ::util::unique<int> a(12);
    stringstream ss;
    a.ToBinary(ss);
    ASSERT(ss.str() == "\x18");

    ::util::unique<int> b;
    b.FromBinary(ss);
    ASSERT(a == b);
  }

  { ::util::unique<string> a("abcd");
    stringstream ss;
    a.ToBinary(ss);
    ASSERT(ss.str() == "\x4""abcd");

    ::util::unique<string> b;
    b.FromBinary(ss);
    ASSERT(a == b);
  }
}

void JSONSerialization() {
  string actual_json = "{\"s\":\"S\",\"us\":\"US\",\"i\":1,\"ui\":2}";
  tTest t1;
  t1.FromJSON(actual_json);

  string serialized_json =  serial::Serializer::ToJSON(t1);
  ASSERT(actual_json == serialized_json);

  tTest t2;
  t2.FromJSON(t1.ToJSON());
  ASSERT(t1 == t2);

  tTest t3;
  serial::Serializer::FromJSON(serial::Serializer::ToJSON(t1), &t3);
  ASSERT(t1 == t3);

  {
    ::util::unique<int> a(12);
    stringstream ss;
    a.ToJSON(ss);
    ASSERT(ss.str() == "12");

    ::util::unique<int> b;
    b.FromJSON(ss);
    ASSERT(a == b);
  }

  { ::util::unique<string> a("abcd");
    stringstream ss;
    a.ToJSON(ss);
    ASSERT(ss.str() == "\"abcd\"");

    ::util::unique<string> b;
    b.FromJSON(ss);
    ASSERT(a == b);
  }
}

void RPCSerialization() {
  string actual_json = "{\"s\":\"S\",\"us\":\"US\",\"i\":1,\"ui\":2}";
  tTest t1;
  t1.FromJSON(actual_json);

  string serialized_json =  serial::Serializer::ToJSON(t1);
  ASSERT(actual_json == serialized_json);

  tTest t2;
  t2.FromBinary(t1.ToBinary());
  ASSERT(t1 == t2);

  tTest t3;
  serial::Serializer::FromBinary(serial::Serializer::ToBinary(t1), &t3);
  ASSERT(t1 == t3);
}

/*void CSVSerialization() {
  string actual_csv = "\"S\",\"U S\",1,2";
  tTest t1;
  t1.FromCSV(actual_csv, ',', false);
  ASSERT("U S", t1.us.get());
  ASSERT(t1.i == 1);
  ASSERT(t1.ui == 2);

  string serialized_csv =  t1.ToCSV(',', false);
  ASSERT(actual_csv, serialized_csv);

  tTest t2;
  t2.FromCSV(t1.ToCSV(',', false), ',', false);
  ASSERT(t1 == t2);
}*/
#endif

}

int main() {
  Sanity();
  WrapClassType();
  WrapNonClassType();
  ParseStream();
  Find();

#ifdef R77_USE_SERIALIZER
  BinarySerialization();
  JSONSerialization();
  RPCSerialization();
#endif

  return 0;
}
