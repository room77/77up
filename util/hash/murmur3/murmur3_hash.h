// **** Begin Room77 Specific Portion ****

// Murmur hash has pretty good comparomise between speed and hash quality (but
// still not a cryptographically strong hasing approach).
// It is about 6 times slower than our string hash which is not as high quality.
// Source code is open source. We only modify it for a convenience function and
// change the header include from .cc file to make it suitable for our paths.

// **** End Room77 Specific Portion ****

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef _PUBLIC_UTIL_HASH_MURMUR3_MURMUR3_HASH_H_
#define _PUBLIC_UTIL_HASH_MURMUR3_MURMUR3_HASH_H_

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER)

typedef unsigned char uint8_t;
typedef unsigned long uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else	// defined(_MSC_VER)

#include <stdint.h>

#endif  // _PUBLIC_UTIL_HASH_MURMUR3_MURMUR3_HASH_H_

//-----------------------------------------------------------------------------

void MurmurHash3_x86_32  ( const void * key, int len, uint32_t seed, void * out );

void MurmurHash3_x86_128 ( const void * key, int len, uint32_t seed, void * out );

void MurmurHash3_x64_128 ( const void * key, int len, uint32_t seed, void * out );

//-----------------------------------------------------------------------------

// **** Begin Room77 Specific Portion ****

#include <string>

inline uint32_t MurmurHash3_32(const std::string& str) {
  uint32_t ret;
  MurmurHash3_x86_32(&str[0], str.size(), 0, &ret);
  return ret;
}

// **** End Room77 Specific Portion ****


#endif  // _PUBLIC_UTIL_HASH_MURMUR3_MURMUR3_HASH_H_
