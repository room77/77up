// Copyright 2013 B. Uygar Oztekin

#ifndef _PUBLIC_UTIL_STORE_CONVERT_STORE_CONVERTER_H_
#define _PUBLIC_UTIL_STORE_CONVERT_STORE_CONVERTER_H_

#include <cassert>
#include <memory>
#include <string>
#include "default_converter.h"
#include "../store.h"

///////////////////////////////////////////////////////////////////////////////
//
// This is a store that wraps another store with different key/data types.
// It transparently translates keys/data to/from what the end-user desires,
// from/to what the store expects.
//
// Note that most low level stores require keys/data to be strings, but in
// general, we may want to use other types (e.g. structs, ints, etc).
// It is desirable to have transparent translation capabilities without end-user
// having to serialize/deserialize before/after each interaction with the store.
// This is where this class comes into play.
//
// Let's say we want to store the data in an efficient string->string store
// (e.g. sst or leveldb), but we want end-user code to keep on using our own key
// and data types (e.g. int as keys, your struct as data). This layer is used to
// automagically do the translation for you. From your point of view, the store
// you deal with is an int -> your struct store.
//
// SerializerConverter uses binary serialization. Key and data converters can be
// specified indepdently if desired. See SerializerConverter and StoreConverter
// template parameters.
//
// Note that since keys are translated to a new data type, it is quite possible
// that original key ordering may be different than translated key ordering.
// As a result, irrespective of the properties of the underlying store, the
// wrapper store may NOT necessarily support:
//
// - sorted forward iteration
// - lower_bound
// - uppoer bound
//
// This class' lower_bound / upper_bound implementation delegates the decision
// to enable / disable them to ConvertKey::is_ordered boolean. If converter
// declares that it preserves key ordering, they are enabled.
//
///////////////////////////////////////////////////////////////////////////////

namespace store {

template<class UserKey,           class UserData,
         class StoreKey = string, class StoreData = string,
         class ConvertKey  = convert::DefaultConverter<UserKey,  StoreKey>,
         class ConvertData = convert::DefaultConverter<UserData, StoreData>
>
class StoreConverter : public Store<UserKey, UserData> {
 public:
  using Parent       = Store<UserKey, UserData>;
  using Child        = Store<StoreKey, StoreData>;
  using key_type     = typename Parent::key_type;
  using data_type    = typename Parent::data_type;
  using size_type    = typename Parent::size_type;
  using value_type   = typename Parent::value_type;
  using iterator     = typename Parent::iterator;
  template<class R> using result = result::Result<R>;

  // Delegates all operations to the underlying store's iterator.
  class StoreConverterIterator : public Parent::CachingIterator {
   public:
    StoreConverterIterator(typename Child::iterator it, const Child& store) : it_(it), store_(store) { Update(); }

    virtual bool operator==(const typename Parent::IteratorBase& it) const {
      auto& rhs = dynamic_cast<const StoreConverterIterator&>(it);
      return it_ == rhs.it_;
    }
    virtual result<bool> operator++() { result<bool> ret = (++it_); if (ret) Update(); return ret; }
    virtual result<bool> operator--() { result<bool> ret = (--it_); if (ret) Update(); return ret; }

   private:
    void Update() {
      if (it_ == store_.end()) return;
      this->cache_.reset(new value_type(ConvertKey()(it_->first), ConvertData()(it_->second)));
    }
    typename Child::iterator it_;
    const Child& store_;
  };

  // Construct using a mutable shared proxy of the underlying store.
  StoreConverter(typename Child::mutable_shared_proxy store) : store_(store) { ASSERT(store_.get()); }

  // Create the mutable shared proxy from id and use it.
  StoreConverter(const string& id) : StoreConverter(typename Child::mutable_shared_proxy(Child::make_shared(id))) {}

  // Basic methods.
  virtual iterator          begin() const { return make_iterator(store_->begin()); }
  virtual iterator          end()   const { return make_iterator(store_->end()); }
  virtual result<bool>      empty() const { return store_->empty(); }
  virtual result<size_type> size()  const { return store_->size(); }

  // Lookups.
  virtual iterator find (const key_type& k) const { return make_iterator(store_->find(make_key(k))); }

  // lower_bound and upper_bound would produce wrong results if the key ordering
  // is not preserved by the converter. Return "not supported" in that case.
  virtual iterator lower_bound (const key_type& k) const {
    return ConvertKey::is_ordered ?
        make_iterator(store_->lower_bound(make_key(k))) :
        typename Parent::not_supported_iterator();
  }

  virtual iterator upper_bound (const key_type& k) const {
    return ConvertKey::is_ordered ?
        make_iterator(store_->upper_bound(make_key(k))) :
        typename Parent::not_supported_iterator();
  }

  // Mutators.
  virtual result<bool> insert(const value_type& v) { return store_->insert(make_value_type(v)); }
  virtual result<bool> update(const value_type& v) { return store_->update(make_value_type(v)); }
  virtual result<bool> erase (const key_type& k)   { return store_->erase(make_key(k)); }
  virtual result<bool> clear ()                    { return store_->clear(); }

 protected:
  typename Child::key_type make_key(const key_type& k) const {
    return ConvertKey()(k);
  }
  typename Child::value_type make_value_type(const value_type& v) const {
    return { ConvertKey()(v.first), ConvertData()(v.second) };
  }
  iterator make_iterator(typename Child::iterator it) const {
    return iterator(new StoreConverterIterator(it, *store_), it.error(), it.supported());
  }

 private:
  typename Child::mutable_shared_proxy store_;
};

}

#endif  // _PUBLIC_UTIL_STORE_CONVERT_STORE_CONVERTER_H_
