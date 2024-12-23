
#include "lrucache.h"

namespace StarryPurple {
template<class KeyType, class ValueType, size_t capacity>
LRUCache<KeyType, ValueType, capacity>::~LRUCache() {
  for(ListType *cur = begin, *del; cur != nullptr;) {
    del = cur;
    cur = cur->nxt;
    delete del;
  }
}

template<class KeyType, class ValueType, size_t capacity>
void LRUCache<KeyType, ValueType, capacity>::insert(const KeyType &key, const ValueType &value) {
  if(page_count == 0) {
    page_count = 1;
    begin = end = new ListType(key, value, nullptr, nullptr);
    node_map.insert({key, end});
    return;
  }
  if(node_map.contains(key)) {
    ListType *cur = node_map[key];
    cur->value = value;
    if(cur == end) return;
    if(cur == begin)
      begin = cur->nxt;
    else cur->prev->nxt = cur->nxt;
    cur->nxt->prev = cur->prev;
    cur->prev = end;
    cur->nxt = nullptr;
    end = end->nxt = cur;
    return;
  }
  page_count++;
  end = end->nxt = new ListType(key, value, end, nullptr);
  node_map.insert({key, end});
  if(page_count > capacity) {
    ListType *del = begin;
    KeyType key = begin->key;
    begin = begin->nxt;
    begin->prev = nullptr;
    delete del;
    node_map.erase(key);
    --page_count;
  }
}

template<class KeyType, class ValueType, size_t capacity>
std::pair<ValueType, bool> LRUCache<KeyType, ValueType, capacity>::find(const KeyType &key) {
  if(node_map.contains(key)) {
    return {node_map[key]->value, true};
  }
  return {ValueType(), false};
}
} // namespace StarryPurple
