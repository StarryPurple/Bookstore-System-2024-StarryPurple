#ifndef INSOMNIA_MULTIMAP_TPP
#define INSOMNIA_MULTIMAP_TPP

#include "insomnia_multimap.h"


template<class KeyType, class ValueType, size_t capacity, int degree>
Insomnia::BlocklistMultimap<KeyType, ValueType, capacity, degree>::~BlocklistMultimap() {
  if(is_open) close();
}

template<class KeyType, class ValueType, size_t capacity, int degree>
void Insomnia::BlocklistMultimap<KeyType, ValueType, capacity, degree>::open(const std::string &prefix) {
  if(is_open) close();
  bool is_exist = multimap_fstream.open(prefix + "_blocklist.bsdat");
  if(is_exist)
    multimap_fstream.read_info(begin_ptr);
  else {
    NodeType begin_node;
    begin_node.node_size = 0;
    begin_ptr = multimap_fstream.allocate(begin_node);
  }
  is_open = true;
}

template<class KeyType, class ValueType, size_t capacity, int degree>
void Insomnia::BlocklistMultimap<KeyType, ValueType, capacity, degree>::close() {
  if(!is_open) return;
  multimap_fstream.write_info(begin_ptr);
  multimap_fstream.close();
  is_open = false;
}

template<class KeyType, class ValueType, size_t capacity, int degree>
void Insomnia::BlocklistMultimap<KeyType, ValueType, capacity, degree>::insert(const KeyType &key, const ValueType &value) {
  KVType kv_pair = {key, value};
  NodePtr cur_ptr = begin_ptr;
  NodeType cur_node; multimap_fstream.read(cur_node, cur_ptr);
  if(cur_node.node_size == 0) {
    // empty
    cur_node.node_size = 1;
    cur_node.kv[0] = kv_pair;
    multimap_fstream.write(cur_node, cur_ptr);
    return;
  }
  while(true) {
    if(kv_pair > cur_node.kv[cur_node.node_size - 1]) {
      if(cur_node.nxt.isnull()) {
        cur_node.kv[cur_node.node_size] = kv_pair;
        ++cur_node.node_size;
        multimap_fstream.write(cur_node, cur_ptr);
        break;
      }
      cur_ptr = cur_node.nxt;
      multimap_fstream.read(cur_node, cur_ptr);
      continue;
    }
    int l = 0, r = cur_node.node_size - 1;
    while(l < r) {
      int mid = (l + r) >> 1;
      if(kv_pair > cur_node.kv[mid]) l = mid + 1;
      else r = mid;
    }
    if(kv_pair == cur_node.kv[l]) return;
    for(int i = cur_node.node_size; i > l; --i)
      cur_node.kv[i] = cur_node.kv[i - 1];
    cur_node.kv[l] = kv_pair;
    ++cur_node.node_size;
    multimap_fstream.write(cur_node, cur_ptr);
    break;
  }
  if(cur_node.node_size >= degree) split(cur_node, cur_ptr);
}

template<class KeyType, class ValueType, size_t capacity, int degree>
void Insomnia::BlocklistMultimap<KeyType, ValueType, capacity, degree>::erase(const KeyType &key, const ValueType &value) {
  KVType kv_pair = {key, value};
  NodePtr cur_ptr = begin_ptr;
  NodeType cur_node; multimap_fstream.read(cur_node, cur_ptr);
  if(cur_node.node_size == 0) return; // empty
  while(true) {
    if(kv_pair > cur_node.kv[cur_node.node_size - 1]) {
      if(cur_node.nxt.isnull()) return;
      cur_ptr = cur_node.nxt;
      multimap_fstream.read(cur_node, cur_ptr);
      continue;
    }
    int l = 0, r = cur_node.node_size - 1;
    while(l < r) {
      int mid = (l + r) >> 1;
      if(kv_pair > cur_node.kv[mid]) l = mid + 1;
      else r = mid;
    }
    if(kv_pair != cur_node.kv[l]) return;
    --cur_node.node_size;
    for(int i = l; i < cur_node.node_size; ++i)
      cur_node.kv[i] = cur_node.kv[i + 1];
    cur_node.kv[cur_node.node_size] = {KeyType(), ValueType()};
    multimap_fstream.write(cur_node, cur_ptr);
    break;
  }
  if(cur_node.node_size < degree / 2) try_average(cur_node, cur_ptr);
}

template<class KeyType, class ValueType, size_t capacity, int degree>
std::vector<ValueType>
Insomnia::BlocklistMultimap<KeyType, ValueType, capacity, degree>::operator[](const KeyType &key) {
  std::vector<ValueType> res;
  NodePtr cur_ptr = begin_ptr;
  NodeType cur_node; multimap_fstream.read(cur_node, cur_ptr);
  if(cur_node.node_size == 0) return res; // empty
  bool is_recording = false;
  while(true) {
    if(is_recording) {
      for(int i = 0; i < cur_node.node_size; ++i) {
        if(cur_node.kv[i].first == key)
          res.push_back(cur_node.kv[i].second);
        else return res;
      }
    } else if(key <= cur_node.kv[cur_node.node_size - 1].first) {
      int l = 0, r = cur_node.node_size - 1;
      while(l < r) {
        int mid = (l + r) >> 1;
        if(key <= cur_node.kv[mid].first) r = mid;
        else l = mid + 1;
      }
      if(cur_node.kv[l].first != key) return res;
      is_recording = true;
      for(int i = l; i < cur_node.node_size; ++i) {
        if(cur_node.kv[i].first == key)
          res.push_back(cur_node.kv[i].second);
        else return res;
      }
    }
    if(cur_node.nxt.isnull()) return res;
    cur_ptr = cur_node.nxt;
    multimap_fstream.read(cur_node, cur_ptr);
  }
}

template<class KeyType, class ValueType, size_t capacity, int degree>
void Insomnia::BlocklistMultimap<KeyType, ValueType, capacity, degree>::split(
  NodeType &left_node, NodePtr &left_ptr) {
  NodeType right_node;
  int left_size = left_node.node_size / 2, right_size = left_node.node_size - left_size;
  KVType empty_kv = {KeyType(), ValueType()};
  for(int i = 0; i < right_size; ++i) {
    right_node.kv[i] = left_node.kv[left_size + i];
    left_node.kv[left_size + i] = empty_kv;
  }
  left_node.node_size = left_size;
  right_node.node_size = right_size;
  right_node.nxt = left_node.nxt;
  right_node.prev = left_ptr;
  left_node.nxt = multimap_fstream.allocate(right_node);
  multimap_fstream.write(left_node, left_ptr);
}

template<class KeyType, class ValueType, size_t capacity, int degree>
void Insomnia::BlocklistMultimap<KeyType, ValueType, capacity, degree>::try_average(
  NodeType &node, NodePtr &node_ptr) {
  NodeType left_node, right_node;
  bool is_left_exist = !node.prev.isnull(), is_right_exist = !node.nxt.isnull();
  if(is_left_exist) {
    multimap_fstream.read(left_node, node.prev);
    if(left_node.node_size < degree / 2) {
      merge(left_node, node, node.prev, node_ptr);
      return;
    }
  }
  if(is_right_exist) {
    multimap_fstream.read(right_node, node.nxt);
    if(right_node.node_size < degree / 2) {
      merge(node, right_node, node_ptr, node.nxt);
      return;
    }
  }
  if(is_left_exist && is_right_exist) {
    if(left_node.node_size > right_node.node_size) {
      average_from_left(left_node, node, node.prev, node_ptr);
      return;
    } else {
      average_from_right(node, right_node, node_ptr, node.nxt);
      return;
    }
  }
  if(is_left_exist) {
    average_from_left(left_node, node, node.prev, node_ptr);
    return;
  }
  if(is_right_exist) {
    average_from_right(node, right_node, node_ptr, node.nxt);
    return;
  }
  // only one node remains. ignore.
}

template<class KeyType, class ValueType, size_t capacity, int degree>
void Insomnia::BlocklistMultimap<KeyType, ValueType, capacity, degree>::average_from_left(
  NodeType &left_node, NodeType &right_node, NodePtr &left_ptr, NodePtr &right_ptr) {
  int total_size = left_node.node_size + right_node.node_size;
  int left_size = total_size / 2, right_size = total_size - left_size;
  int diff = right_size - right_node.node_size;
  for(int i = right_node.node_size - 1; i >= 0; --i)
    right_node.kv[i + diff] = right_node.kv[i];
  KVType empty_kv = {KeyType(), ValueType()};
  for(int i = 0; i < diff; ++i) {
    right_node.kv[i] = left_node.kv[left_size + i];
    left_node.kv[left_size + i] = empty_kv;
  }
  left_node.node_size = left_size;
  right_node.node_size = right_size;
  multimap_fstream.write(left_node, left_ptr);
  multimap_fstream.write(right_node, right_ptr);
}

template<class KeyType, class ValueType, size_t capacity, int degree>
void Insomnia::BlocklistMultimap<KeyType, ValueType, capacity, degree>::average_from_right(
  NodeType &left_node, NodeType &right_node, NodePtr &left_ptr, NodePtr &right_ptr) {
  int total_size = left_node.node_size + right_node.node_size;
  int left_size = total_size / 2, right_size = total_size - left_size;
  int diff = left_size - left_node.node_size;
  for(int i = 0; i < diff; ++i)
    left_node.kv[left_node.node_size + i] = right_node.kv[i];
  for(int i = 0; i < right_size; ++i)
    right_node.kv[i] = right_node.kv[diff + i];
  KVType empty_kv = {KeyType(), ValueType()};
  for(int i = right_size; i < right_node.node_size; ++i)
    right_node.kv[i] = empty_kv;
  left_node.node_size = left_size;
  right_node.node_size = right_size;
  multimap_fstream.write(left_node, left_ptr);
  multimap_fstream.write(right_node, right_ptr);
}

template<class KeyType, class ValueType, size_t capacity, int degree>
void Insomnia::BlocklistMultimap<KeyType, ValueType, capacity, degree>::merge(
  NodeType &left_node, NodeType &right_node, NodePtr &left_ptr, NodePtr &right_ptr) {
  int total_size = left_node.node_size + right_node.node_size;
  for(int i = 0; i < right_node.node_size; ++i)
    left_node.kv[left_node.node_size + i] = right_node.kv[i];
  left_node.node_size = total_size;
  if(!right_node.nxt.isnull()) {
    NodeType rr_node;
    multimap_fstream.read(rr_node, right_node.nxt);
    rr_node.prev = left_ptr;
    multimap_fstream.write(rr_node, right_node.nxt);
  }
  left_node.nxt = right_node.nxt;
  multimap_fstream.write(left_node, left_ptr);
  multimap_fstream.free(right_ptr);
}




#endif // INSOMNIA_MULTIMAP_TPP