// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#ifndef _PUBLIC_UTIL_TEMPLATES_ITERATOR_H_
#define _PUBLIC_UTIL_TEMPLATES_ITERATOR_H_

#include <iterator>

namespace util {
namespace tl {

// Implements a simple circular iterator on top of any iterator.
// This iterator cannot iterate over the complete list. It will always reduce
// the range of the container by 1. See associated test for examples.
template<typename T>
class circular_iterator :
    public std::iterator<std::forward_iterator_tag, typename T::value_type> {
 public:
  circular_iterator(T& t) : iter_(t.begin()), begin_(t.begin()), end_(t.end()) {
    SharedCtor();
  }

  circular_iterator(typename T::iterator b, typename T::iterator e)
      : iter_(b), begin_(b), end_(e) {
    SharedCtor();
  };

  circular_iterator(T& t, typename T::iterator iter)
      : iter_(iter), begin_(t.begin()), end_(t.end()) {
    SharedCtor();
  }

  circular_iterator(typename T::iterator b, typename T::iterator e,
                    typename T::iterator iter)
      : iter_(iter), begin_(b), end_(e) {
    SharedCtor();
  }

  void SharedCtor() {
    if (iter_ == end_) iter_ = begin_;
  }

  circular_iterator<T>& operator ++() {
    ++iter_;
    if (iter_ == end_) iter_ = begin_;

    return(*this);
  }

  circular_iterator<T> operator ++(int) {
    circular_iterator<T> t = *this;
    ++(*this);
    return t;
  }

  bool operator ==(const circular_iterator<T>& rhs) const {
    return iter_ == rhs.iter_;
  }

  bool operator !=(const circular_iterator<T>& rhs) const {
    return ! operator==(rhs);
  }

  const typename T::value_type& operator*() const { return (*iter_); }
  typename T::value_type& operator*() { return (*iter_); }

 protected:
  typename T::iterator   iter_;
  typename T::iterator   begin_;
  typename T::iterator   end_;
};

// Implements a circular range on given range. The behavior is tuned for loops.
// Please verify behavior before using it in any other way.
// Iterates over the range [b, e).
// If the range has begin and end pointing to the same element, it will loop
// over the entire container. This class is not thread safe.
//
template<typename T>
class circular_range {
 public:
  circular_range(T& t, typename T::iterator b, typename T::iterator e)
      : range_begin_(t, b), range_end_(t, e), iter_(t, b) {
    skip_once_ = range_begin_ == range_end_;
  }

  circular_range(const circular_iterator<T>& b, const circular_iterator<T>& e)
      : range_begin_(b), range_end_(e), iter_(b) {
    skip_once_ = range_begin_ == range_end_;
  }

  circular_range& begin() { return *this; }
  circular_range& end() { return *this; }

  // Returns the range iterator.
  circular_iterator<T>& range_begin() { return range_begin_; }
  circular_iterator<T>& range_end() { return range_end_; }
  circular_iterator<T>& iter() { return iter_; }

  circular_range& operator ++() {
    ++iter_;
    return(*this);
  }

  circular_iterator<T> operator ++(int) {
    circular_range<T> range = *this;
    ++(*this);
    return range;
  }

  bool operator ==(const circular_iterator<T>& rhs) const {
    // In case the range was equal we ignore the first request for equality.
    // This behavior is tuned for loops.
    // Please verify behavior before using it in any other way.
    bool eq = iter_ == rhs;
    if (eq && skip_once_) {
      skip_once_ = false;
      return false;
    }
    return eq;
  }

  // This function is supposed to work with loops. Please verify your use case
  // before using this function for anything else.
  bool operator ==(const circular_range& rhs) const {
    return operator==(rhs.range_end_);
  }

  bool operator !=(const circular_iterator<T>& rhs) const {
    return ! operator==(rhs);
  }

  bool operator !=(const circular_range& rhs) const {
    return ! operator==(rhs);
  }

  const typename T::value_type& operator*() const { return (*iter_);}
  typename T::value_type& operator*() {return (*iter_);}

 protected:
  circular_iterator<T>   range_begin_;
  circular_iterator<T>   range_end_;
  circular_iterator<T>   iter_;
  mutable bool skip_once_ = false;
};

}  // namespace tl
}  // namespace util


#endif  // _PUBLIC_UTIL_TEMPLATES_ITERATOR_H_
