// Copyright 2012 Room77, Inc.
// Author: Uygar Oztekin

// This server knows how to launch and monitor tasks of the form
// Task<function<bool()>> (default template parameter).

// See task_server_example.cc for an example on how to use this library.

#ifndef _PUBLIC_UTIL_THREAD_TASK_SERVER_H_
#define _PUBLIC_UTIL_THREAD_TASK_SERVER_H_

#include <algorithm>
#include <chrono>
#include <mutex>
#include <deque>

#include "util/thread/task.h"
#include "util/network/method/server_method.h"

namespace task {

// Keeps track of a few past runs of a task along with return value.
// RPC method returns history (up to last 10 runs) for the requested task_id
// (or every task id it knows of if task_id is empty).
// start and finish timestamps, ids, and return status is rememebered.
// Finish time of 0 means that the task is still running.
class History : public network::ServerMethod {
 public:
  struct tHistory {
    string task_id;
    uint64_t start;
    uint64_t finish;
    bool success;
    SERIALIZE(task_id*1 / start*2 / finish*3 / success*4);
  };
  struct Request  { string task_id; SERIALIZE(task_id*1); };
  struct Reply    { vector<tHistory> history; SERIALIZE(history*1); };

  string operator()(const Request& req, Reply *reply) const {
    lock_guard<std::recursive_mutex> l(mutex());
    if (req.task_id.empty()) {
      for (auto it = history().begin(); it != history().end(); ++it)
        for (auto& p : it->second) reply->history.push_back(p.second);
    }
    else {
      auto it = history().find(req.task_id);
      for (auto& p : it->second) reply->history.push_back(p.second);
    }
    return "";
  }

  // Update a runnning task's entry. finish = 0 means task is still running.
  static void Update(const string& task_id, bool status, uint64_t start, uint64_t finish = Now()) {
    lock_guard<std::recursive_mutex> l(mutex());
    history()[task_id][start] = tHistory{task_id, start, finish, status};
    if (history()[task_id].size() > 10)
      history()[task_id].erase(history()[task_id].begin());
  }

  static uint64_t Now() {
    return chrono::duration_cast<chrono::microseconds>(
        chrono::high_resolution_clock::now().time_since_epoch()).count();
  }

 private:
  static recursive_mutex& mutex() {
    static recursive_mutex m;
    return m;
  }
  static map<string, map<uint64_t, tHistory>>& history() {
    static map<string, map<uint64_t, tHistory>> h;
    return h;
  };
};

// Start a given task. Returns started=true if the task is started by this call.
class Start : public network::ServerMethod {
 public:
  struct Request  { string task_id; SERIALIZE(task_id*1); };
  struct Reply    { bool   started; SERIALIZE(started*1); };

  string operator()(const Request& req, Reply *reply) const {
    auto task = Task<>::make_shared(req.task_id);
    if (!task.get()) return "Task id " + req.task_id + " not found";
    reply->started = task->run(launch::async);
    if (!reply->started) return "";
    string id = req.task_id;
    uint64_t now = History::Now();
    History::Update(id, false, now, 0);
    thread([id, now, task]{ History::Update(id, task->wait(), now); }).detach();
    return "";
  }
};

// Can be used to see if a given task is currently running.
class Status : public network::ServerMethod {
 public:
  struct Request  { string task_id; SERIALIZE(task_id*1); };
  struct Reply    { bool   running; SERIALIZE(running*1); };

  string operator()(const Request& req, Reply *reply) const {
    auto task = Task<>::make_shared(req.task_id);
    if (!task.get()) return "Task id " + req.task_id + " not found";
    reply->running = task.use_count() > 1 && task->Locked();
    return "";
  }
};

}

#endif  // _PUBLIC_UTIL_THREAD_TASK_SERVER_H_
