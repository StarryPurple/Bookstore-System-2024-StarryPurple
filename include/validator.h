/**
 *
 *
 */
#pragma once
#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "bookstore_exceptions.h"

namespace StarryPurple {
template<class T>
class Validator {
private:
  T v;
  bool is_rev = false;
public:
  Validator &And = *this;
  Validator &Or = *this;
  Validator &but = *this;
  Validator(const T &val);
  Validator &toBe(const T &other);
  template<class U, class ... Args>
  Validator &toBeOneOf(const U &other, const Args &... args );
  template<class U>
  Validator &toBeOneOf(const U &other);
  Validator &lesserEqual(const T &other);
  Validator &greaterEqual(const T &other);
  Validator &Not();
  template<class CharT>
  Validator &toBeConsistedOf(const std::basic_string<CharT> &str);
  template<class CharT>
  Validator &toMatch(const std::basic_string<CharT> &str);
};

template<class T>
Validator<T> expect(const T &val);

} // namespace StarryPurple

#endif // VALIDATOR_H