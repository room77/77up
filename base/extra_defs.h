#ifndef _PUBLIC_BASE_EXTRA_DEFS_H_
#define _PUBLIC_BASE_EXTRA_DEFS_H_

// radius of earth in kilometers
const double EARTH_RADIUS = 6371.0;

const double PI = 3.14159265358979323846;

// number definitions

#define INFINITY_INT (0x7fffffff)
#define INFINITY_FLOAT (static_cast<float>(1e+30))
#define INFINITY_PRICE (static_cast<tPrice>(1e+30))
#define IS_FINITE_INT(a) ((a) != -INFINITY_INT && (a) != INFINITY_INT)
#define IS_FINITE_PRICE(a) ((a) < INFINITY_PRICE)

// MOD: similar to % operator, but handles negative numbers properly and
//      always return positive values within [0, b)
//      -- Note: "b" must be a positive integer; "a" can be any integer
#define MOD(a,b) (((a) >= 0) ? ((a) % (b)) : ((-((-(a)) % (b)) + (b)) % (b)))

#define IS_DIGIT(s) ((s) >= '0' && (s) <= '9')
#define IS_LETTER(s) (((s) >= 'A' && (s) <= 'Z') || ((s) >= 'a' && (s) <= 'z'))
#define IS_HEXDIGIT(s) (((s) >= '0' && (s) <= '9') || \
                      ((s) >= 'A' && (s) <= 'F') || ((s) >= 'a' && (s) <= 'f'))
#define IS_ALPHANUMERIC(s) (IS_DIGIT(s) || IS_LETTER(s))
#define IS_ALPHANUMERIC_INTL(s) (IS_DIGIT(s) || IS_LETTER(s) || \
                                 (((s) & 128) != 0))

#endif
