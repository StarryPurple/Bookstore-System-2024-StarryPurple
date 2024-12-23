#pragma once
#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <utility>

namespace StarryPurple {
template<class KeyType, class ValueType, size_t capacity>
class LRUCache {
  struct ListType {
    KeyType key;
    ValueType value;
    ListType *prev, *nxt;
  };
  std::unordered_map<KeyType, ListType *> node_map;
  size_t page_count = 0;
  ListType *begin = nullptr, *end = nullptr;
  // filled begin, filled end
public:
  LRUCache() = default;
  ~LRUCache();
  void insert(const KeyType &key, const ValueType &value);
  std::pair<ValueType, bool> find(const KeyType &key);
};

} // namespace StarryPurple

#endif // LRU_CACHE_H