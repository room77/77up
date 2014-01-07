// Copyright 2013 Room77, Inc.
// Author: Uygar Oztekin

#include "init.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <chrono>
#include <mutex>
#include <set>

#include "base/args/args.h"
#include "base/logging.h"
#include "util/thread/task_list.h"

// Comma delimited list of init groups to include / exclude.
FLAG_string(init_include, "", "Init groups to be included. Empty means all.");
FLAG_string(init_exclude, "", "Init groups to be excluded. Empty means none.");

FLAG_string(exit_include, "", "Exit groups to be included. Empty means all.");
FLAG_string(exit_exclude, "", "Exit groups to be excluded. Empty means none.");

FLAG_bool(init_async, true, "Launch parallel by default. False means serial.");
FLAG_bool(exit_async, true, "Launch parallel by default. False means serial.");

// No need to expose this class outside (yet). Let's keep it inside an unnamed
// namespace.
namespace {

struct InitTaskList : public task::TaskList<function<void()>> {
  static const string Description() { return "Init"; }
};

struct ExitTaskList : public task::TaskList<function<void()>> {
  static const string Description() { return "Exit"; }
};

template<class task_list_type>
class TaskRunner {
  typedef Factory<task_list_type, pair<int, string>> factory_type;

  struct TaskInfo {
    int priority;
    string group;
    string location;
    int duration;
  };

 public:
  static function<void()> TimedRunner(const string& id, function<void()> f,
                                      const char* loc, int p) {
    static mutex m;
    lock_guard<mutex> l(m);
    auto it = PriorityToTaskInfo().insert(make_pair(make_pair(p, id),
                                                    TaskInfo{p, id, loc, 0}));
    ASSERT(it != PriorityToTaskInfo().end());
    TaskInfo& task_info = it->second;
    return [f, &task_info]{
      auto t0 = chrono::high_resolution_clock::now();
      f();
      auto t1 = chrono::high_resolution_clock::now();
      task_info.duration = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();
    };
  }

  static bool Add(const string& id, function<void()> f,
                  const char* loc = "", int priority = 0) {
    auto key = make_pair(priority, id);
    factory_type::bind(key, []{ return new task_list_type(); });
    auto t = factory_type::make_shared(key);
    factory_type::pin(t);
    ASSERT(t.get());
    t->push_back(TimedRunner(id, f, loc, priority));
    return true;
  }

  // Run everything in parallel.
  static bool Run(bool async = true) {
    return Run(set<string>(), set<string>(), async);
  }

  static bool Run(const string& includes, const string& excludes, bool async = true) {
    return Run(FlagToSet(includes), FlagToSet(excludes), async);
  }

  // Run everything in includes but not in excludes. If includes is empty, Run
  // everything that is registered but not excluded.
  static bool Run(const set<string>& includes, const set<string>& excludes, bool async = true) {
    map<int, vector<pair<int, string>>> groups;
    stringstream output;
    // TODO: there is a bug in GCC where factory_type::keys<set>() does not
    // compile in this context. Once the bug is fixed we should no longer use
    // append_keys().
    //
    // Works: Factory<InitTaskList,   pair<int, string>>::keys<set>();
    // Fails: Factory<task_list_type, pair<int, string>>::keys<set>();
    //
    // Even though the template parameter is actually InitTaskList.
    //
    set<pair<int, string>> keys;
    factory_type::append_keys(keys);
    for (const auto& k : keys) {
      if ((!k.second.empty() && k.second[0] == '!') ||
            ((includes.empty() || includes.find(k.second) != includes.end()) &&
            excludes.find(k.second) == excludes.end())) {
        groups[k.first].push_back(k);
        output << k.second << "(" << k.first << ") ";
      }
    }
    VLOG(2) << task_list_type::Description() + " groups to Run: " << output.str();
    return Run(groups, async);
  }

  // Run only the specified ids. Each priority group is run in parallel.
  static bool Run(const map<int, vector<pair<int, string>>>& ids, bool async = true) {
    deque<typename factory_type::shared_proxy> task_groups;
    bool ret = true;
    for (auto& p : ids) {
      for (const auto& id : p.second) {
        auto t = factory_type::make_shared(id);
        ASSERT(t.get())
            << task_list_type::Description() + " group does not exist: '"
            << id.second << "'";
        ret &= t->run(async ? launch::async : launch::deferred);
      }
      for (const auto& id : p.second) {
        auto t = factory_type::make_shared(id);
        ASSERT(t.get());
        t->wait();
      }
    }
    VLOG(3) << task_list_type::Description() + " done";
    return ret;
  }

  static string Summary() {
    stringstream ss;
    constexpr int max_size = 20;
    ss << setw(max_size) << "Group" << " Priority  Time  Location" << endl;
    for (auto& p : PriorityToTaskInfo()) {
      auto& t = p.second;
      string group = t.group;
      if (group.size() > max_size) group = group.substr(0, max_size);
      ss << fixed << setw(max_size) << group << " " << setw(5) << t.priority
         << " " << setw(8) << setprecision(3) << t.duration / 1000.0
         << "s " << t.location << endl;
    }
    return ss.str();
  }

 protected:
  static set<string> FlagToSet(const string& flag) {
    set<string> tokens;
    stringstream ss(flag);
    for (;;) {
      string token;
      getline(ss, token, ',');
      if (ss.fail()) break;
      tokens.insert(token);
    }
    return tokens;
  }

  static multimap<pair<int, string>, TaskInfo>& PriorityToTaskInfo() {
    static multimap<pair<int, string>, TaskInfo> m;
    return m;
  }
};

}  // namespace

namespace init {

bool InitAdd(const string& grp, const char* loc, function<void()> f) {
  return TaskRunner<InitTaskList>::Add(grp, f, loc, 0);
}

bool InitAdd(const string& grp, const char* loc, int p, function<void()> f) {
  return TaskRunner<InitTaskList>::Add(grp, f, loc, p);
}

bool ExitAdd(const string& grp, const char* loc, function<void()> f) {
  return TaskRunner<ExitTaskList>::Add(grp, f, loc, 0);
}

bool ExitAdd(const string& grp, const char* loc, int p, function<void()> f) {
  return TaskRunner<ExitTaskList>::Add(grp, f, loc, p);
}

}  // namespace init

void InitRun() {
  auto t0 = chrono::high_resolution_clock::now();
  TaskRunner<InitTaskList>::Run(gFlag_init_include, gFlag_init_exclude, gFlag_init_async);
  auto t1 = chrono::high_resolution_clock::now();
  auto d = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count() / 1000.0;
  VLOG(d > 1 ? 2 : 3) << "InitRun() finished in " << d << "s\n"
      << TaskRunner<InitTaskList>::Summary();
}

void ExitRun() {
  auto t0 = chrono::high_resolution_clock::now();
  TaskRunner<ExitTaskList>::Run(gFlag_exit_include, gFlag_exit_exclude, gFlag_exit_async);
  auto t1 = chrono::high_resolution_clock::now();
  auto d = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count() / 1000.0;
  VLOG(d > 1 ? 2 : 3) << "ExitRun() finished in " << d << "s\n"
      << TaskRunner<ExitTaskList>::Summary();
}
