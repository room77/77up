// Copyright 2013 B. Uygar Oztekin, Pramod Gupta
// Author: pramodg@room77.com (Pramod Gupta)
//         oztekin@room77.com (Uygar Oztekin)

#ifndef _PUBLIC_THIRD_PARTY_OZTEKIN_FACTORY_MUTABLE_PTR_H_
#define _PUBLIC_THIRD_PARTY_OZTEKIN_FACTORY_MUTABLE_PTR_H_

// Utility class to remove the constness of the data stored in Ptr.
// Note: Be careful while using this class as now we have mutable
// pointer/reference to T. Make sure T is thread safe.
template<typename T, typename Ptr=std::shared_ptr<const T> >
class mutable_ptr : public Ptr {
 public:
  mutable_ptr() : Ptr() {}
  mutable_ptr(const Ptr& ptr) : Ptr(ptr) {}

  mutable_ptr& operator=(const Ptr& ptr) {
    Ptr::operator=(ptr);
    return *this;
  }

  typename std::add_lvalue_reference<T>::type operator*() const {
    return const_cast<T&>(Ptr::operator*());
  }

  T* operator->() const {
    return const_cast<T*>(Ptr::operator->());
  }

  T* get() const { return const_cast<T*>(Ptr::get()); }
};

#endif  // _PUBLIC_THIRD_PARTY_OZTEKIN_FACTORY_MUTABLE_PTR_H_
