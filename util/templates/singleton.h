// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_UTIL_TEMPLATES_SINGLETON_H_
#define _PUBLIC_UTIL_TEMPLATES_SINGLETON_H_

#include <memory>

namespace util {
namespace tl {

// Generate a singleton for a class of type T.
template <typename T>
class Simpleton {
 public :
  static T& Instance() {
    static T simple;
    return simple;
  }

 protected:
  Simpleton() {}
  Simpleton(const Simpleton&) = delete;
  Simpleton& operator=(const Simpleton&) = delete;
};

// Generate a singleton for a class of type T. It creates the object on heap.
template <typename T>
class Singleton {
 public :
  static T& Instance() {
    static std::unique_ptr<T> single(new T);
    return *single;
  }

 protected:
  Singleton() {}
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;
};

}  // namespace tl
}  // namespace util

#endif  // _PUBLIC_UTIL_TEMPLATES_SINGLETON_H_
