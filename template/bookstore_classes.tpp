#ifndef BOOKSTORE_CLASSES_TPP
#define BOOKSTORE_CLASSES_TPP

#include "bookstore_classes.h"

template<int capacity>
BookStore::ConstStr<capacity>::ConstStr() {
  memset(storage, 0, sizeof(storage));
  len = 0;
}

template<int capacity>
BookStore::ConstStr<capacity>::ConstStr(const std::string &str) {
  expect(str.size()).lesserEqual(capacity);
  len = str.size();
  strcpy(storage, str.c_str()); // for '\0'
}

template<int capacity>
BookStore::ConstStr<capacity>::ConstStr(const ConstStr &other) {
  len = other.len;
  strcpy(storage, other.storage, len + 1);
}

template<int capacity>
bool BookStore::ConstStr<capacity>::operator==(const ConstStr &other) const {
  if(len != other.len)
    return false;
  for(int i = 0; i < len; ++i)
    if(storage[i] != other.storage[i])
      return false;
  return true;
}

template<int capacity>
bool BookStore::ConstStr<capacity>::operator!=(const ConstStr &other) const {
  return !operator==(other);
}

template<int capacity>
bool BookStore::ConstStr<capacity>::empty() const {
  return len == 0;
}



#endif // BOOKSTORE_CLASSES_TPP