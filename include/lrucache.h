#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <map>
#include <utility>

namespace StarryPurple {

// todo: change std::map to std::unordered_map
//       std::hash(KeyType) is to be required.
template<class KeyType, class ValueType, size_t capacity>
class LRUCache {
  struct ListType {
    KeyType key;
    ValueType value;
    ListType *prev, *nxt;
  };
  std::map<KeyType, ListType *> node_map;
  size_t page_count = 0;
  ListType *begin = nullptr, *end = nullptr;
  // filled begin, filled end
public:
  LRUCache() = default;
  ~LRUCache();
  void insert(const KeyType &key, const ValueType &value);
  std::pair<ValueType, bool> find(const KeyType &key); // if found, automatically call an insert(key, value)
};

} // namespace StarryPurple

#include "lrucache.tpp"

#endif // LRU_CACHE_H