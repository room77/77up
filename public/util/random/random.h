// Copyright 2013 Room77, Inc.
// Author: akearney@room77.com (Andy Kearney), pramodg@room77.com (Pramod Gupta)

// Utility function for generating random numbers/strings.

#ifndef _PUBLIC_UTIL_RANDOM_RANDOM_H_
#define _PUBLIC_UTIL_RANDOM_RANDOM_H_

#include <chrono>
#include <functional>
#include <limits>
#include <random>
#include <sstream>
#include <type_traits>

#include "base/defs.h"

namespace util {
namespace random {

// Alias for Random function.
template<typename IntType = int>
using RandomNumberFromUniformIntDistributionFunc = std::function<IntType ()>;

template<typename IntType = int>
RandomNumberFromUniformIntDistributionFunc<IntType> GetRandomNumberFromUniformIntDistributionFunc(
    IntType a = 0, IntType b = std::numeric_limits<IntType>::max()) {
  return std::bind(std::uniform_int_distribution<IntType>(a, b),
                   std::mt19937(std::random_device()()));
}

// Alias for Random function.
template<typename FloatingPointType = float>
using RandomNumberFromUniformFloatingPointDistributionFunc =
    std::function<FloatingPointType ()>;

template<typename FloatingPointType = float>
RandomNumberFromUniformFloatingPointDistributionFunc<FloatingPointType>
GetRandomNumberFromUniformFloatingPointDistributionFunc(
    FloatingPointType a = 0,
    FloatingPointType b = std::numeric_limits<FloatingPointType>::max()) {
  return std::bind(std::uniform_real_distribution<FloatingPointType>(a, b),
                   std::mt19937(std::random_device()()));
}

// Returns a random number in the range [a, b].
template<typename IntType = int, IntType a = 0, IntType b = std::numeric_limits<IntType>::max()>
IntType GetRandomNumberInRange() {
  static auto random_func = GetRandomNumberFromUniformIntDistributionFunc<IntType>(a, b);
  return random_func();
}

// Returns a random number in the range [a, b].
// floating point types cannot be used an nontype template params
// so we cannot use the same method as for random integers
template<typename FloatingPointType = float>
struct GetRandomFloatingPointNumberInRange {
  GetRandomFloatingPointNumberInRange(
      FloatingPointType a = 0,
      FloatingPointType b = std::numeric_limits<FloatingPointType>::max()) :
      random_func_(
          GetRandomNumberFromUniformFloatingPointDistributionFunc<FloatingPointType>(a, b))
  {}

  FloatingPointType operator()() {
    return random_func_();
  }

 private:
  RandomNumberFromUniformFloatingPointDistributionFunc<FloatingPointType> random_func_;
};

// Returns a random number in range [0, std::numeric_limits<IntType>::max()]
template<typename IntType = int>
IntType GetRandomNumber() {
  return GetRandomNumberInRange<IntType>();
}

// Returns a unique hash of type TypeReturned by adding timestamp and rand() to the input string.
// TypeReturned is useful in ensuring that the hash is of a particular size.
// Make sure that TypeInput supports  '<<' operator.
// Make sure that TypeReturned is numeric.
template<typename TypeRandom = int, typename TypeInput = string>
string GetUniqueString(const TypeInput& input = TypeInput()) {
  stringstream ss;
  ss << input << GetRandomNumber<TypeRandom>() << chrono::duration_cast<chrono::microseconds>(
      chrono::high_resolution_clock::now().time_since_epoch()).count();
  return ss.str();
}

// Returns a unique hash of type TypeReturned by adding timestamp and rand() to the input string.
// TypeReturned is useful in ensuring that the hash is of a particular size.
// Make sure that TypeInput supports  '<<' operator.
// Make sure that TypeReturned is numeric.
template<typename TypeReturned = size_t, typename TypeRandom = int, typename TypeInput = string>
typename make_unsigned<TypeReturned>::type GetUniqueHash(const TypeInput& input = TypeInput()) {
  return static_cast<typename make_unsigned<TypeReturned>::type>(std::hash<string>()(
      GetUniqueString<TypeRandom, TypeInput>(input)));
}

// Utility function to return the hash in string form.
template<typename TypeReturned = size_t, typename TypeRandom = int, typename TypeInput = string>
string GetUniqueHashString(const TypeInput& input = TypeInput()) {
  return to_string(GetUniqueHash<TypeReturned, TypeRandom, TypeInput>(input));
}

template<class Distribution>
typename std::remove_reference<Distribution>::type::result_type GetRandomValueFromDistribution(
    Distribution&& distribution,
    typename std::random_device::result_type seed = std::random_device()()) {
  auto urng = std::mt19937(seed);
  return distribution(urng);
}

}  // namespace random
}  // namespace util


#endif  // _PUBLIC_UTIL_RANDOM_RANDOM_H_
