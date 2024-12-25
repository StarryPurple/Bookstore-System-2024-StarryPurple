/** validator.cpp
 *
 */
#include "validator.h"

#include <regex>


template<class T>
StarryPurple::Validator<T> expect(const T &val) {
  return Validator<T>(val);
}

template<class T>
StarryPurple::Validator<T>::Validator(const T &val) {
  v = val;
}

template<class T>
StarryPurple::Validator<T>
&StarryPurple::Validator<T>::Not() {
  is_rev = !is_rev;
  return *this;
}

template<class T>
StarryPurple::Validator<T>
&StarryPurple::Validator<T>::toBe(const T &other) {
  bool res = (v == other);
  if(res ^ is_rev)
    return *this;
  throw ValidatorException();
}

template<class T>
template<class U, class... Args>
StarryPurple::Validator<T>
&StarryPurple::Validator<T>::toBeOneOf(const U &other, const Args &... args) {
  bool res = (std::is_same_v<T, U> && v == other);
  if(res ^ is_rev)
    return *this;
  return toBeOneOf(args ...);
}

template<class T>
template<class U>
StarryPurple::Validator<T>
&StarryPurple::Validator<T>::toBeOneOf(const U &other) {
  bool res = (std::is_same_v<T, U> && v == other);
  if(res ^ is_rev)
    return *this;
  throw ValidatorException();
}

template<class T>
StarryPurple::Validator<T>
&StarryPurple::Validator<T>::lesserEqual(const T &other) {
  bool res = (v <= other);
  if(res ^ is_rev)
    return *this;
  throw ValidatorException();
}

template<class T>
StarryPurple::Validator<T>
&StarryPurple::Validator<T>::greaterEqual(const T &other) {
  bool res = (v >= other);
  if(res ^ is_rev)
    return *this;
  throw ValidatorException();
}

template<class T>
template<class CharT>
StarryPurple::Validator<T>
&StarryPurple::Validator<T>::toBeConsistedOf(const std::basic_string<CharT> &str) {
  static_assert(std::is_same_v<T, std::basic_string<CharT>>);
  typename std::basic_string<CharT>::size_type n;
  for(CharT ch: v) {
    n = str.find(ch);
    if(n == std::basic_string<CharT>::npos)
      throw ValidatorException();
  }
  return *this;
}

template<class T>
template<class CharT>
StarryPurple::Validator<T>
&StarryPurple::Validator<T>::toMatch(const std::basic_string<CharT> &str) {
  static_assert(std::is_same_v<T, std::basic_string<CharT>>);
  std::basic_regex<CharT> regex(str);
  bool res = std::regex_match(v, regex);
  if(res ^ is_rev)
    return *this;
  throw ValidatorException();
}


