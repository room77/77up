// Need to define quick_exit and at_quick_exit for systems that do not contain
// the implementation in libc

#ifdef __APPLE__
#ifndef _THIRD_PARTY_LIBC_STDLIB_QUICK_EXIT_H_
#define _THIRD_PARTY_LIBC_STDLIB_QUICK_EXIT_H_

int at_quick_exit(void (*func)(void));
void quick_exit(int status);

#endif  // _THIRD_PARTY_LIBC_STDLIB_QUICK_EXIT_H_
#endif  // __APPLE__
