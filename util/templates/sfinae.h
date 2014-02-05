// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// This code is copied from
// http://stackoverflow.com/questions/257288/is-it-possible-to-write-a-c-template-to-check-for-a-functions-existence/257382#257382
// with only a few minor changes.

/*
// Check for member variable x:
  class MyClass { int x; };
  ...
  CREATE_MEMBER_VAR_CHECK(x);
  bool has_var_x = has_member_var_x<MyClass>::value;

// Check for member class x:
  class MyClass { Class x {...}; };
  ...
  CREATE_MEMBER_CLASS_CHECK(x);
  bool has_class_x = has_member_class_x<MyClass>::value;

// Check for member union x:
  class MyClass { union x {...}; };
  ...
  CREATE_MEMBER_UNION_CHECK(x);
  bool has_union_x = has_member_union_x<MyClass>::value;

// Check for member enum x:
  class MyClass { union x {...}; };
  ...
  CREATE_MEMBER_ENUM_CHECK(x);
  bool has_enum_x = has_member_enum_x<MyClass>::value;

// Check for member function void x(). This works with inherited members but does
// work with overloaded members.
// (Note: Func signature MUST have T as template variable):
  class MyClass { void x(); };
  ...
  CREATE_MEMBER_FUNC_CHECK(x);
  bool has_func_sig_void__x = has_member_func_sig_void__x<MyClass>::value;

// Check for any member function x regardless of signature:
  class MyClass { ... };
  ...
  CREATE_MEMBER_CHECKS(x);
  bool has_any_func_x = has_member_func_x<MyClass>::value;

// Check for member x in a given class.
// Could be var, func, class, union, or enum:
  class MyClass { ... };
  ...
  CREATE_MEMBER_CHECK(x);
  bool has_x = has_member_x<MyClass>::value;

// Check for member function void x(). This does not work with inherited members.
// (Note: Func signature MUST have T as template variable):
  class MyClass { void x(); };
  ...
  CREATE_MEMBER_FUNC_EXACT_SIG_CHECK(x);
  bool has_member_func_exact_sig_x =
      has_member_func_exact_sig_x<MyClass, void ()>::value;

// Check for member function void x(). This works with inherited members.
// However, this also returns true for implictly castable types.
// (Note: Func signature MUST have T as template variable):
  class MyClass { void x(); };
  ...
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(x);
  bool has_func_sig_x = has_member_func_sig_x<MyClass, void()>::value;
*/


#ifndef _PUBLIC_UTIL_TEMPLATES_SFINAE_H_
#define _PUBLIC_UTIL_TEMPLATES_SFINAE_H_

#include <type_traits>
#include <tuple>

namespace sfinae {

template <typename... Args> struct ambiguate : public std::tuple<Args...> {};

template<typename A, typename = void>
struct got_type : std::false_type {};

template<typename A>
struct got_type<A> : std::true_type {
    typedef A type;
};

template<typename T, T>
struct sig_check : std::true_type {};

template<typename Alias, typename AmbiguitySeed>
struct has_member {
    template<typename C> static char ((&f(decltype(&C::value))))[1];
    template<typename C> static char ((&f(...)))[2];

    //Make sure the member name is consistently spelled the same.
    static_assert((sizeof(f<AmbiguitySeed>(0)) == 1),
            "Member name specified in AmbiguitySeed is different from member "
            "name specified in Alias, or wrong Alias/AmbiguitySeed has "
            "been specified.");

    static const bool value = sizeof(f<Alias>(0)) == 2;
};

}  // namespace sfinae

// Check for any member with given name, whether var, func, class, union, enum.
#define CREATE_MEMBER_CHECK(member)                                         \
template<typename T, typename = std::true_type>                             \
struct Alias_##member;                                                      \
                                                                            \
template<typename T>                                                        \
struct Alias_##member <T, std::integral_constant<bool,                      \
    sfinae::got_type<decltype(&T::member)>::value>> {                       \
  static const decltype(&T::member) value;                                  \
};                                                                          \
                                                                            \
struct AmbiguitySeed_##member { char member; };                             \
                                                                            \
template<typename T>                                                        \
struct has_member_##member {                                                \
  static const bool value = sfinae::has_member<                             \
      Alias_##member<sfinae::ambiguate<T, AmbiguitySeed_##member>>,         \
      Alias_##member<AmbiguitySeed_##member>>::value; }                     \

// Check for member variable with given name.
#define CREATE_MEMBER_VAR_CHECK(var_name)                                   \
template<typename T, typename = std::true_type>                             \
struct has_member_var_##var_name : std::false_type {};                      \
                                                                            \
template<typename T>                                                        \
struct has_member_var_##var_name<T, std::integral_constant<bool,            \
    !std::is_member_function_pointer<decltype(&T::var_name)>::value>> : std::true_type {} \

// Check for member class with given name.
#define CREATE_MEMBER_CLASS_CHECK(class_name)                               \
template<typename T, typename = std::true_type>                             \
struct has_member_class_##class_name : std::false_type {};                  \
                                                                            \
template<typename T>                                                        \
struct has_member_class_##class_name<T, std::integral_constant<bool,        \
    std::is_class<typename sfinae::got_type<                                \
        typename T::class_name>::type>::value>> : std::true_type {}         \

// Check for member union with given name.
#define CREATE_MEMBER_UNION_CHECK(union_name)                               \
template<typename T, typename = std::true_type>                             \
struct has_member_union_##union_name : std::false_type {};                  \
                                                                            \
template<typename T>                                                        \
struct has_member_union_##union_name<T, std::integral_constant<bool,        \
    std::is_union<typename sfinae::got_type<                                \
        typename T::union_name>::type>::value>> : std::true_type {}         \

// Check for member enum with given name.
#define CREATE_MEMBER_ENUM_CHECK(enum_name)                                 \
template<typename T, typename = std::true_type>                             \
struct has_member_enum_##enum_name : std::false_type {};                    \
                                                                            \
template<typename T>                                                        \
struct has_member_enum_##enum_name<T, std::integral_constant<bool,          \
    std::is_enum<typename sfinae::got_type<                                 \
        typename T::enum_name>::type>::value>> : std::true_type {}          \

// Check for function with given name, any signature.
#define CREATE_MEMBER_FUNC_CHECK(func)                                      \
template<typename T, typename = std::true_type>                             \
struct has_member_func_##func : std::false_type {};                         \
                                                                            \
template<typename T>                                                        \
struct has_member_func_##func<T, std::integral_constant<bool,               \
    std::is_member_function_pointer<decltype(&T::func)>::value>> : std::true_type {} \

//Create all the checks for one member.  Does NOT include func sig checks.
#define CREATE_MEMBER_CHECKS(member)                                       \
CREATE_MEMBER_CHECK(member);                                               \
CREATE_MEMBER_VAR_CHECK(member);                                           \
CREATE_MEMBER_CLASS_CHECK(member);                                         \
CREATE_MEMBER_UNION_CHECK(member);                                         \
CREATE_MEMBER_ENUM_CHECK(member);                                          \
CREATE_MEMBER_FUNC_CHECK(member)

// Check for member function with given name AND soft signature
// (allows implicitly castable types). Note:
// This works with inherited members.
#define CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(func)                             \
template<typename T, typename F, typename = std::true_type>                 \
struct has_member_func_sig_##func : std::false_type {};                     \
                                                                            \
template<typename T, typename _Res, typename... _ArgTypes>                  \
struct has_member_func_sig_##func<T, _Res (_ArgTypes...),                   \
    std::integral_constant<bool, std::is_same<_Res,                         \
        typename std::remove_cv<                                            \
            decltype(((typename std::remove_reference<T>::type*)0)->func(   \
                std::declval<_ArgTypes>()...))>::type>::value>>             \
    : std::true_type {}                                                     \

// Check for member function with given name AND soft signature
// (allows implicitly castable types). Note:
// This works with inherited members.
// This is useful when the argument or return types are elements in the type
// itself.
#define CREATE_MEMBER_FUNC_FIXED_SOFT_SIG_CHECK(res, func, ...)             \
CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(func);                                    \
template<typename T, typename = std::true_type>                             \
struct has_member_func_fixed_sig_##func : std::false_type {};               \
                                                                            \
template<typename T>                                                        \
struct has_member_func_fixed_sig_##func<T, std::integral_constant<bool,     \
    has_member_func_sig_##func<T, res(__VA_ARGS__)>::value>>                \
    : std::true_type {}                                                     \

// Check for member function with given name AND signature. Note:
// This does not work with inherited members.
#define CREATE_MEMBER_FUNC_EXACT_SIG_CHECK(func)                            \
template<typename T, typename F, typename = std::true_type>                 \
struct has_member_func_exact_sig_##func : std::false_type {};               \
                                                                            \
template<typename T, typename _Res, typename... _ArgTypes>                  \
struct has_member_func_exact_sig_##func<T, _Res (_ArgTypes...),             \
    std::integral_constant<bool,                                            \
        sfinae::sig_check<_Res (T::*)(_ArgTypes...),                        \
            static_cast<_Res (T::*)(_ArgTypes...)>(&T::func)>::value>>      \
    : std::true_type {};                                                    \
                                                                            \
template<typename T, typename _Res, typename... _ArgTypes>                  \
struct has_member_func_exact_sig_##func<T, _Res (_ArgTypes...) const,       \
    std::integral_constant<bool,                                            \
        sfinae::sig_check<_Res (T::*)(_ArgTypes...) const,                  \
            static_cast<_Res (T::*)(_ArgTypes...) const>(&T::func)>::value>> \
    : std::true_type {}                                                     \

// Checks if the member has a type.
#define CREATE_MEMBER_TYPE_CHECK(__type)                                    \
template<typename T, typename = std::true_type>                             \
struct has_member_type_##__type : std::false_type {};                       \
                                                                            \
template<typename T>                                                        \
struct has_member_type_##__type<T, std::integral_constant<bool,             \
    sfinae::got_type<typename T::__type>::value>>                           \
    : std::true_type {}                                                     \

#endif  // _PUBLIC_UTIL_TEMPLATES_SFINAE_H_
