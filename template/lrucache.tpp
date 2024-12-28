#ifndef LRU_CACHE_TPP
#define LRU_CACHE_TPP

#include "lrucache.h"

template<class KeyType, class ValueType, size_t capacity>
StarryPurple::LRUCache<KeyType, ValueType, capacity>::~LRUCache() {
  for(ListType *cur = begin, *del; cur != nullptr;) {
    del = cur;
    cur = cur->nxt;
    delete del;
  }
}

template<class KeyType, class ValueType, size_t capacity>
void StarryPurple::LRUCache<KeyType, ValueType, capacity>::insert(const KeyType &key, const ValueType &value) {
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
    KeyType del_key = begin->key;
    begin = begin->nxt;
    begin->prev = nullptr;
    delete del;
    node_map.erase(del_key);
    --page_count;
  }
}

template<class KeyType, class ValueType, size_t capacity>
std::pair<ValueType, bool> StarryPurple::LRUCache<KeyType, ValueType, capacity>::find(const KeyType &key) {
  if(node_map.contains(key)) {
    ValueType value = node_map[key]->value;
    insert(key, value);
    return {value, true};
  }
  return {ValueType(), false};
}

#endif // LRU_CACHE_TPP