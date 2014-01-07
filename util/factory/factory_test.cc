// Copyright 2011 B. Uygar Oztekin
// Copied and modified from Uygar's personal libraries.

#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include <deque>
#include "factory.h"

using namespace std;

struct Base {
  virtual ~Base() {}
  virtual string Who() const { return "Base"; }
  virtual string Params() const { return params_.str(); }
  stringstream params_;
};

struct Derived1 : public Base {
  Derived1(int n) {
    params_ << n;
    cout << "Creating Derived1 with params " << Params() << endl;
  }
  virtual string Who() const  { return "Derived1"; }
};

struct Derived2 : public Base {
  Derived2(const string& p1, const string& p2) {
    params_  << p1 << " " << p2;
    cout << "Creating Derived2 with params " << Params() << endl;
  }
  virtual string Who() const { return "Derived2"; }
};

struct Derived3 : public Base {
  Derived3(const string& p1, int p2, char p3) {
    params_ << p1 << " " << p2 << " " << p3;
    cout << "Creating Derived3 with params " << Params() << endl;
  }
  virtual string Who() const { return "Derived3"; }
};

int d1_flag = -1;


namespace {
auto reg_d1 = Factory<Base>::bind("d1", [](){ return new Derived1(d1_flag); });
// register an alias to d1.
auto reg_d1_alias = Factory<Base>::alias("d1_alias", "d1");
// In this case, params are not mutable, so it may be fine not to use refs.
auto reg_d2 = Factory<Base>::bind("d2", [](){ return new Derived2("hello", "world"); });

// Register a parameterized version with 3 parameters (string, int and char).
auto reg_p1 = Factory<Base, string, string, int, char>::bind("p1", "default", 10, 't', [](const string& str, int i, char c){ return new Derived3(str, i, c); });
}

void TestBasicFunctionality() {
  cout << "\nTesting basic functionality\n" << endl;
  assert(Factory<Base>::make_shared("NO_SUCH_ID").get() == nullptr);
  assert(Factory<Base>::make_unique("NO_SUCH_ID").get() == nullptr);
  {
    // Pretend the commandline flags is just parsed and we have 1 (instead of -1).
    d1_flag = 1;
    auto d1 = Factory<Base>::make_shared("d1");
    assert(d1.get());
    assert(d1.use_count() == 1);
    // Make sure that we used the new value for the parameter.
    assert(d1->Params() == "1");
    assert(d1->Who() == "Derived1");

    // This new value for d1_flag should not be reflected in the subsequent
    // shared instance since d1 is alive.
    d1_flag = 100;
    // Create a new d1 proxy and make sure that it uses the shared instance.
    auto another_d1 = Factory<Base>::make_shared("d1");
    assert(another_d1.get());
    assert(d1.use_count() == 2);
    assert(another_d1.use_count() == 2);
    auto yet_another_d1 = Factory<Base>::make_shared("d1_alias");
    assert(yet_another_d1.get());
    assert(d1.use_count() == 3);
    assert(another_d1.use_count() == 3);
    assert(yet_another_d1.use_count() == 3);
    // Make sure that we used the new value for the parameter.
    assert(d1->Params() == "1");
    assert(another_d1->Who() == "Derived1");
    assert(d1.get() == another_d1.get());

    // Now, let's create a unique proxy for d1 and make sure that it uses the
    // new flag.
    auto unique_d1 = Factory<Base>::make_unique("d1");
    assert(unique_d1.get());
    // Make sure that we used the new value for the parameter.
    assert(unique_d1->Params() == "100");
    assert(unique_d1->Who() == "Derived1");
    // They should not be using the same instance.
    assert(d1.get() != unique_d1.get());
    // Make sure two unique_ptrs to the same instance are not equal.
    assert(d1.get() != Factory<Base>::make_unique("d1").get());
  }
  {
    auto d2 = Factory<Base>::make_shared("d2");
    assert(d2.get());
    assert(d2->Who() == "Derived2");
    assert(d2->Params() == "hello world");
  }
}

void TestParameterizedFunctionality() {
  cout << "\nTesting parameterized functionality\n" << endl;
  // Create one using default parameters.
  {
    auto p1 = Factory<Base, string, string, int, char>::make_unique("p1");
    assert(p1.get());
    assert(p1->Who() == "Derived3");
    assert(p1->Params() == "default 10 t");
  }
  // Create one with fully specified on-the-fly parameters.
  {
    auto p1 = Factory<Base, string, string, int, char>::make_unique("p1", "unique1", 1, 'a');
    assert(p1.get());
    assert(p1->Who() == "Derived3");
    assert(p1->Params() == "unique1 1 a");
  }
  // Override a subset of the parameters.
  {
    auto p1 = Factory<Base, string, string, int, char>::make_updated("p1", "test1", 1);
    auto p2 = Factory<Base, string, string, int, char>::make_shared("p1", "test2", 2);
    auto p3 = Factory<Base, string, string, int, char>::make_shared("p1", "test2", 2);
    auto p4 = Factory<Base, string, string, int, char>::make_shared("p1");
    auto p5 = Factory<Base, string, string, int, char>::make_shared("p1", "test5");
    assert(p1.get() && p1->Who() == "Derived3");
    assert(p2.get() && p2->Who() == "Derived3");
    assert(p3.get() && p3->Who() == "Derived3");
    assert(p4.get() && p4->Who() == "Derived3");
    assert(p5.get() && p5->Who() == "Derived3");
    assert(p1.use_count() == 1);
    assert(p2.use_count() == 2);  // p2 and p3 is supposed to share instance.
    assert(p3.use_count() == 2);
    assert(p1->Params() == "test1 1 t");
    assert(p2->Params() == "test2 2 t");
    assert(p3->Params() == "test2 2 t");
    assert(p4->Params() == "default 10 t");
    assert(p5->Params() == "test5 10 t");
  }
}

void TestKeyEnumerationFunctionality() {
  cout << "\nTesting key enumeration functionality\n" << endl;
  {
    auto keys = Factory<Base>::keys<vector>();
    cout << "Registered keys for Factory<Base>:\n";
    for (const auto& k : keys) cout << k << endl;
    cout << endl;
    assert(keys.size() == 3);
  }
  {
    auto keys = Factory<Base, string, string, int, char>::keys<deque>();
    cout << "Registered keys for Factory<Base, string, string, int, char>:\n";
    for (const auto& k : keys) cout << k << endl;
    cout << endl;
    assert(keys.size() == 1);
  }
  {
    auto key_values = Factory<Base, string, string, int, char>::key_values<map>();
    cout << "Registered keys and values for Factory<Base, string, string, int, char>:\n";
    for (const auto& k : key_values)
      cout << k.first << " " << get<0>(k.second) << " " << get<1>(k.second)
           << " " << get<2>(k.second) << endl;
    assert(key_values.size() == 1);
  }
}

void TestPinUnpinFunctionality() {
  cout << "\nTesting pin / unpin functionality\n" << endl;
  // Create an instance and pin it.
  d1_flag = 1;
  {
    auto d1 = Factory<Base>::make_shared("d1");
    assert(d1.get());
    assert(d1.use_count() == 1);
    // Pin it twice. We will need to unpin twice as well.
    Factory<Base>::pin(d1);
    Factory<Base>::pin(d1);

    assert(d1.get());
    assert(d1->Params() == "1");
    assert(d1->Who() == "Derived1");
    assert(d1.use_count() == 2);
  }
  d1_flag = 100;
  // Make sure it is pinned and we used 1 for flag.
  {
    auto d1 = Factory<Base>::make_shared("d1");
    assert(d1.get());
    assert(d1->Params() == "1");
    assert(d1->Who() == "Derived1");
    assert(d1.use_count() == 2);

  }
  // Unpin once.
  {
    auto d1 = Factory<Base>::make_shared("d1");
    Factory<Base>::unpin(d1);
    assert(d1.get());
    assert(d1->Params() == "1");
    assert(d1->Who() == "Derived1");
    assert(d1.use_count() == 2);
  }

  // Unpin one more time.
  {
    auto d1 = Factory<Base>::make_shared("d1");
    Factory<Base>::unpin(d1);
    assert(d1.get());
    assert(d1->Params() == "1");
    assert(d1->Who() == "Derived1");
    assert(d1.use_count() == 1);
  }

  // Now we should not have an active instance. Let's make sure we get 100.
  {
    auto d1 = Factory<Base>::make_shared("d1");
    assert(d1.get());
    assert(d1->Params() == "100");
    assert(d1->Who() == "Derived1");
    assert(d1.use_count() == 1);
  }

  // This should cause an assertion:
  // auto d1 = Factory<Base>::make_shared("d1");
  // Factory<Base>::unpin(d1);
}

void TestMakeUpdatedFunctionality() {
  d1_flag = 100;
  auto d1 = Factory<Base>::make_shared("d1");
  assert(d1.get());
  assert(d1->Params() == "100");
  assert(d1->Who() == "Derived1");

  // Update the flag.
  d1_flag = 1000;

  // Since we did not call make_updted, this should still get the old one.
  auto d1_2 = Factory<Base>::make_shared("d1");
  assert(d1_2.get());
  assert(d1_2->Params() == "100");
  assert(d1_2->Who() == "Derived1");

  // Call make_updated to force a new instance using the new value.
  auto d1_3 = Factory<Base>::make_updated("d1");
  assert(d1_3.get());
  assert(d1_3->Params() == "1000");
  assert(d1_3->Who() == "Derived1");

  // Make sure the new d1 instance gets the updated version.
  auto d1_4 = Factory<Base>::make_shared("d1");
  assert(d1_4.get());
  assert(d1_4->Params() == "1000");
  assert(d1_4->Who() == "Derived1");

  // Make sure old instances are not affected.
  assert(d1.get());
  assert(d1->Params() == "100");
  assert(d1->Who() == "Derived1");
  assert(d1_2.get());
  assert(d1_2->Params() == "100");
  assert(d1_2->Who() == "Derived1");
}

int main() {
  TestBasicFunctionality();
  TestParameterizedFunctionality();
  TestKeyEnumerationFunctionality();
  TestPinUnpinFunctionality();
  TestMakeUpdatedFunctionality();
  return 0;
}
