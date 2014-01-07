// Copyright 2012 Room77, Inc.
// Author: B. Uygar Oztekin

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <thread>
#include "task.h"
#include "task_list.h"

#ifndef ASSERT
#include <cassert>
#define ASSERT assert
#endif

using namespace std;

int gFlag_num_tasks = 10000; // Number of async tasks.

// Note that the parameter is a reference type.
bool TestParam(const string& str) {
  cout << str << endl;
  return true;
}

void PrintNumThreads() {
  ifstream file("/proc/self/status");
  string line;
  for (getline(file, line); !file.fail(); getline(file, line)) {
    stringstream ss;
    ss << line;
    string token;
    ss >> token;
    if (token != "Threads:") continue;
    ss >> token;
    cout << "Number of threads right now: " << token << endl;
  }
}

int main() {
  // Basic test.
  {
    task::Task<> t([]{ cout << "Hello World!" << endl; return true; });
    t.run(launch::deferred);
    t.wait();
  }

  // Test scopes and parameter binding.
  {
    task::Task<> t;
    {
      // This should go away after the scope.
      string str = "Test params via bind";
      t = task::Task<>(std::bind(&TestParam, str));
    }
    t.run(launch::deferred);
    t.wait();

    {
      t = task::Task<>([]{ return TestParam("Test params via lamda");});
    }
    t.run(launch::deferred);  // task is locked but not launched yet (deferred).
    ASSERT(!t.run());         // Make sure it is locked.
    t.wait();                 // launchs and finishes here.
  }

  // Async test.
  {
    vector<shared_ptr<task::Task<>>> tasks;
    int gFlag_num_tasks = 10000;
    cout << "Launching " << gFlag_num_tasks <<" async tasks" << endl;
    vector<int> done(gFlag_num_tasks, 0);
    for (int i = 0; i < gFlag_num_tasks; ++i) {
      shared_ptr<task::Task<>> t(new task::Task<>([&done, i]{
        done[i] = 1;
        stringstream ss;
        ss << "My thread id: " << std::this_thread::get_id() << endl;
        // cout << ss.str();
        this_thread::sleep_for(chrono::milliseconds(300));
        return true;
      }));
      tasks.push_back(t);
      (*tasks.rbegin())->run(launch::async);
    }

    PrintNumThreads();

    cout << "Checking async tasks " << endl;
    for (int i = 0; i < tasks.size(); ++i) {
      tasks[i]->wait();
      ASSERT(done[i] == 1);
    }
  }

  // Task list test.
  {
    vector<int> done(gFlag_num_tasks, 0);
    task::TaskList<> tasks;
    for (int i = 0; i < gFlag_num_tasks; ++i) {
      tasks.push_back([&done, i]{
        done[i] = 1;
        stringstream ss;
        ss << "My thread id: " << std::this_thread::get_id() << endl;
        // cout << ss.str();
        this_thread::sleep_for(chrono::milliseconds(300));
        return true;
      });
    }

    tasks.run(launch::async);

    PrintNumThreads();

    tasks.wait();
    cout << "Checking async tasks " << endl;
    for (int i = 0; i < tasks.size(); ++i) {
      ASSERT(done[i] == 1);
    }
  }
  return 0;
}
