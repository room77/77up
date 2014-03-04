// Copyright 2011 B. Uygar Oztekin
// Copied and modified from Uygar's personal libraries.

// This header contains some convenience functors that can be used via the
// factory mechanism that does not necessarily need to be part of the basic
// factory library.

#ifndef _PUBLIC_THIRD_PARTY_OZTEKIN_FACTORY_FACTORY_EXTRA_H_
#define _PUBLIC_THIRD_PARTY_OZTEKIN_FACTORY_FACTORY_EXTRA_H_

#include <functional>
#include <iostream>
#include <memory>
#include <tuple>

#include "tuple_util.h"

// Helper functor that assumes that the Base class has the following virtual
// methods:
//
// bool Configure(const P& p);
// bool Initialize();
//
// Configure is used to configure the class before the Initialize call. It can
// be called multiple times to configure with base parameters, override some of
// parameters etc., but should NOT be called after Initialize is called.
// Initialize is useful for expensive initializations (e.g. reading a file),
// or any initializations that depends on configuration parameters.
//
// If either of the functions returns false, instance is destroyed and the
// functor returns nullptr.
template<typename T, typename P>
struct InitializeConfigureConstructor {
  InitializeConfigureConstructor() {}

  // Note: If you use this constructor, make sure you are passing it a persistent (= global/static)
  // variable. We store a pointer and not a copy of the variable. e.g. calling
  // InitializeConfigureConstructor("") is not allowed.
  InitializeConfigureConstructor(const P& p) { params_ = { &p }; }

  virtual ~InitializeConfigureConstructor() {}

  T* operator()() const {
    T* ptr = GetNewObject();
    for (int i = 0; i < params_.size(); ++i) {
      if (!ptr->Configure(*params_[i])) {
        std::cout << typeid(T).name() << ": error parsing params: " << *params_[i] << std::endl;
        delete ptr;
        return nullptr;
      }
    }
    if (!ptr->Initialize()) {
      std::cout << typeid(T).name() << ": error during initialization." << std::endl;
      delete ptr;
      return nullptr;
    }
    return ptr;
  }

  T* operator()(const P& p) const {
    // TODO(pramodg): Verify whether this needs to be thread safe.
    params_.push_back(&p);
    T* res = operator()();
    params_.pop_back();
    return res;
  }

  virtual T* GetNewObject() const { return new T;}

  mutable std::vector<const P*> params_;
};

// This is a subclass of the basic InitializeConfigureConstructor where we also allow some
// specific parameters to be specified while creating a new object. This is useful when there
// are some fields that are needed to init a class but shouldn't be exposed to the user.
template<typename T, typename P,  typename... ConstructorParams>
struct InitializeConfigureWithConstructorParams : public InitializeConfigureConstructor<T, P> {
  typedef std::tuple<ConstructorParams...> ConstructorParamsType;

  InitializeConfigureWithConstructorParams() {}
  using InitializeConfigureConstructor<T, P>::InitializeConfigureConstructor;
  InitializeConfigureWithConstructorParams(const P& p, ConstructorParams... constructor_params)
      : InitializeConfigureConstructor<T, P>(p), constructor_params_(constructor_params...) {}

  virtual ~InitializeConfigureWithConstructorParams() {}

  virtual T* GetNewObject() const {
    return tuple_util::tuple_apply(std::function<T*(ConstructorParams...)>(
        [](ConstructorParams... p) { return new T(p...); }), constructor_params_);
  }

  ConstructorParamsType constructor_params_;
};


#endif  // _PUBLIC_THIRD_PARTY_OZTEKIN_FACTORY_FACTORY_EXTRA_H_
