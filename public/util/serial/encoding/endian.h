// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_UTIL_SERIAL_ENCODING_ENDIAN_H_
#define _PUBLIC_UTIL_SERIAL_ENCODING_ENDIAN_H_

#include <algorithm>

#include "base/defs.h"

namespace serial {
namespace endian {

// Converts Endianness.

inline bool IsBigEndian(void) {
  static union {
    uint32_t i;
    char c;
  } endinaness = { 0x01020304 };
  return endinaness.c == 1;
}

// Swaps the endianness.
template <class T>
void SwapEndianness(T *v) {
  unsigned char *memp = reinterpret_cast<unsigned char*>(v);
  std::reverse(memp, memp + sizeof(T));
}

// Converts Host To Little Endian.
template <class T>
void HToLE(T *v) {
  if (IsBigEndian()) SwapEndianness(v);
}

// Converts Host To Big Endian.
template <class T>
void HToBE(T *v) {
  if (!IsBigEndian()) SwapEndianness(v);
}

// Converts Little Endian To Host.
template <class T>
void LEToH(T *v) {
  if (IsBigEndian()) SwapEndianness(v);
}

// Converts Big Endian To Host.
template <class T>
void BEToH(T *v) {
  if (!IsBigEndian()) SwapEndianness(v);
}

}  // namespace endian
}  // namespace serial


#endif  // _PUBLIC_UTIL_SERIAL_ENCODING_ENDIAN_H_
