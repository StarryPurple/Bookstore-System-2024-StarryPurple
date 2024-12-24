#include "fmultimap.h"

namespace StarryPurple {

template<class KeyType, class ValueType, size_t degree, size_t capacity>
Fmultimap<KeyType, ValueType, degree, capacity>::~Fmultimap() {
  if(is_open)
    close();
}

template<class KeyType, class ValueType, size_t degree, size_t capacity>
void Fmultimap<KeyType, ValueType, degree, capacity>::open(
  const filenameType &name_suffix) {
  inner_fstream.open(name_suffix + "_multimap.bsdat");
  is_open = true;
  inner_fstream.read_info(root_ptr);
}

template<class KeyType, class ValueType, size_t degree, size_t capacity>
void Fmultimap<KeyType, ValueType, degree, capacity>::close() {
  inner_fstream.write_info(root_ptr);
  inner_fstream.close();
  is_open = false;
}

template<class KeyType, class ValueType, size_t degree, size_t capacity>
void Fmultimap<KeyType, ValueType, degree, capacity>::insert(
  const KeyType &key, const ValueType &value) {
  KVPairType kv_pair = std::make_pair(key, value);
  if(root_ptr.isnull()) {

    InnerNode inner_root_node;
    inner_root_node.parent_ptr.setnull();
    inner_root_node.is_leaf = true; inner_root_node.high_kv = kv_pair; inner_root_node.node_size = 1;
    inner_root_node.kv_pairs[0] = kv_pair;
    root_ptr = inner_fstream.allocate(inner_root_node);
    // initialize requires no maintain_size.
    return;
  }

  InnerPtr cur_inner_ptr = root_ptr, parent_ptr; // parent_ptr = "nullptr" = root_ptr.parent_ptr
  InnerNode cur_inner_node; inner_fstream.read(cur_inner_node, cur_inner_ptr);
  size_t pos;
  while(true) {
    if(cur_inner_node.is_leaf && kv_pair > cur_inner_node.high_kv) {

      ++cur_inner_node.node_size;
      cur_inner_node.parent_ptr = parent_ptr;
      cur_inner_node.high_kv = kv_pair;
      cur_inner_node.kv_pairs[cur_inner_node.node_size - 1] = kv_pair;
      inner_fstream.write(cur_inner_node, cur_inner_ptr);
      // node_size modified. Start maintenance.
      maintain_size(cur_inner_ptr, cur_inner_node);
      return;
    }
    if(cur_inner_node.is_leaf) {
      if(cur_inner_node.parent_ptr != parent_ptr) {
        cur_inner_node.parent_ptr = parent_ptr;
        inner_fstream.write(cur_inner_node, cur_inner_ptr);
      }
      size_t l = 0, r = cur_inner_node.node_size - 1;
      while(l < r) {
        size_t mid = (l + r) >> 1;
        if(kv_pair > cur_inner_node.kv_pairs[mid]) l = mid + 1;
        else r = mid;
      }
      pos = l;
      break;
    }
    if(kv_pair > cur_inner_node.high_kv) {
      cur_inner_node.parent_ptr = parent_ptr;
      cur_inner_node.high_kv = kv_pair;
      cur_inner_node.kv_pairs[cur_inner_node.node_size - 1] = kv_pair;
      inner_fstream.write(cur_inner_node, cur_inner_ptr);
      parent_ptr = cur_inner_ptr;
      cur_inner_ptr = cur_inner_node.inner_ptrs[cur_inner_node.node_size - 1];
      inner_fstream.read(cur_inner_node, cur_inner_ptr);
      continue;
    }
    if(cur_inner_node.parent_ptr != parent_ptr) {
      cur_inner_node.parent_ptr = parent_ptr;
      inner_fstream.write(cur_inner_node, cur_inner_ptr);
    }
    size_t l = 0, r = cur_inner_node.node_size - 1;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      if(kv_pair > cur_inner_node.kv_pairs[mid]) l = mid + 1;
      else r = mid;
    }
    parent_ptr = cur_inner_ptr;
    cur_inner_ptr = cur_inner_node.inner_ptrs[l];
    inner_fstream.read(cur_inner_node, cur_inner_ptr);
  }
  if(kv_pair < cur_inner_node.kv_pairs[pos]) {
    ++cur_inner_node.node_size;
    for(size_t i = cur_inner_node.node_size - 1; i > pos; --i)
      cur_inner_node.kv_pairs[i] = cur_inner_node.kv_pairs[i - 1];
    cur_inner_node.kv_pairs[pos] = kv_pair;
    inner_fstream.write(cur_inner_node, cur_inner_ptr);
    // node_size modified. Start maintenance.
    maintain_size(cur_inner_ptr, cur_inner_node);
    return;
  }
  // If code reaches here, it means we're inserting existing kv_pair. ignore it.
}

template<class KeyType, class ValueType, size_t degree, size_t capacity>
void Fmultimap<KeyType, ValueType, degree, capacity>::erase(
  const KeyType &key, const ValueType &value) {
  KVPairType kv_pair = std::make_pair(key, value);
  if(root_ptr.isnull())
    return;
  InnerPtr cur_inner_ptr = root_ptr, parent_ptr; // parent_ptr = root_ptr.parent_ptr = "nullptr"
  InnerNode cur_inner_node; inner_fstream.read(cur_inner_node, cur_inner_ptr);
  if(kv_pair > cur_inner_node.high_kv)
    return; // too large
  size_t pos;
  while(true) {
    if(kv_pair > cur_inner_node.high_kv) {
      return;
    }
    if(cur_inner_node.parent_ptr != parent_ptr) {
      cur_inner_node.parent_ptr = parent_ptr;
      inner_fstream.write(cur_inner_node, cur_inner_ptr);
    }
    size_t l = 0, r = cur_inner_node.node_size - 1;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      if(kv_pair > cur_inner_node.kv_pairs[mid]) l = mid + 1;
      else r = mid;
    }
    if(cur_inner_node.inner_ptrs) {
      pos = l;
      break;
    }
    parent_ptr = cur_inner_ptr;
    cur_inner_ptr = cur_inner_node.inner_ptrs[l];
    inner_fstream.read(cur_inner_node, cur_inner_ptr);
  }
  if(kv_pair < cur_inner_node.kv_pairs[pos])
    return; // kv pair not exist
  for(size_t i = pos; i < cur_inner_node.node_size; ++i)
    cur_inner_node.kv_pairs[i] = cur_inner_node.kv_pairs[i + 1];
  --cur_inner_node.node_size;
  inner_fstream.write(cur_inner_node, cur_inner_ptr);
  // node size modified.
  maintain_size(cur_inner_ptr, cur_inner_node);
}

template<class KeyType, class ValueType, size_t degree, size_t capacity>
std::vector<ValueType> Fmultimap<KeyType, ValueType, degree, capacity>::operator[](
  const KeyType &key) {
  KVPairType kv_pair = std::make_pair(key, ValueType());
  std::vector<ValueType> res;
  if(root_ptr.isnull()) return res;
  InnerPtr cur_inner_ptr = root_ptr, parent_ptr; // parent_ptr = root_node.parent_ptr = "nullptr"
  InnerNode cur_inner_node; inner_fstream.read(cur_inner_node, cur_inner_ptr);
  if(kv_pair > cur_inner_node.high_kv) return res; // key too large
  size_t pos;
  while(true) {
    if(cur_inner_node.parent_ptr != parent_ptr) {
      cur_inner_node.parent_ptr = parent_ptr;
      inner_fstream.write(cur_inner_node, cur_inner_ptr);
    }
    size_t l = 0, r = cur_inner_node.node_size - 1;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      if(kv_pair > cur_inner_node.kv_pairs[mid]) l = mid + 1;
      else r = mid;
    }
    if(cur_inner_node.is_leaf) {
      pos = l;
      break;
    }
    parent_ptr = cur_inner_ptr;
    cur_inner_ptr = cur_inner_node.inner_ptrs[l];
    inner_fstream.read(cur_inner_node, cur_inner_ptr);
  }
  while(true) {
    for(size_t i = pos; i < cur_inner_node.node_size; ++i) {
      if(cur_inner_node.kv_pairs[i].first == key)
        res.push_back(cur_inner_node.kv_pairs[i].second);
      else return res;
    }
    if(cur_inner_node.link_ptr.isnull())
      return res;
    cur_inner_ptr = cur_inner_node.link_ptr;
    inner_fstream.read(cur_inner_node, cur_inner_ptr);
    pos = 0;
  }
}

template<class KeyType, class ValueType, size_t degree, size_t capacity>
void Fmultimap<KeyType, ValueType, degree, capacity>::maintain_size(
  InnerPtr &maintain_ptr, InnerNode &maintain_node) {
  while(true) {
    if(maintain_node.node_size >= degree + 1) { // node_size upper limit
      // here maintain_ptr = root_ptr, maintain_node is the root node.
      if(maintain_ptr == root_ptr) {
        // create a new root.
        InnerNode new_root;
        new_root.is_leaf = false; new_root.high_kv = maintain_node.high_kv;
        // new_root.parent_ptr = "nullptr"
        new_root.node_size = 1;
        new_root.kv_pairs[0] = maintain_node.high_kv;
        new_root.inner_ptrs[0] = maintain_ptr;
        // new_root.link_ptr = "nullptr"
        // You can do the split completely here to reduce two file operations.
        // But I'm lazy now. or to say crazy? So I refuse.
        root_ptr = maintain_node.parent_ptr = inner_fstream.allocate(new_root);
        inner_fstream.write(maintain_node, maintain_ptr);
        split(0, maintain_ptr, maintain_node);
        return;
      }
      // find the split_pos.
      InnerNode parent_node; inner_fstream.read(parent_node, maintain_node.parent_ptr);
      size_t l = 0, r = parent_node.node_size - 1;
      while(l < r) {
        size_t mid = (l + r) >> 1;
        if(maintain_node.high_kv > parent_node.kv_pairs[mid]) l = mid + 1;
        else r = mid;
      }
      assert(maintain_node.high_kv == parent_node.kv_pairs[l]);
      split(l, maintain_ptr, maintain_node);
      continue;
    }
    // if(maintain_node.node_size < degree / 2) {...}
    return;
  }
}

template<class KeyType, class ValueType, size_t degree, size_t capacity>
void Fmultimap<KeyType, ValueType, degree, capacity>::split(
  const size_t split_pos, InnerPtr &split_ptr, InnerNode &split_node) {
  // split a new node and insert it into the right pos.
  size_t left_size = split_node.node_size / 2, right_size = split_node.node_size - left_size;

  InnerNode right_node;
  right_node.is_leaf = split_node.is_leaf; right_node.high_kv = split_node.high_kv;
  right_node.parent_ptr = split_node.parent_ptr; right_node.node_size = right_size;
  right_node.link_ptr = split_node.link_ptr;
  KVPairType empty_kv_pair = std::make_pair(KeyType(), ValueType());
  for(size_t i = 0; i < right_size; ++i) {
    right_node.kv_pairs[i] = split_node.kv_pairs[left_size + i];
    right_node.inner_ptrs[i] = split_node.inner_ptrs[left_size + i];
    split_node.kv_pairs[left_size + i] = empty_kv_pair;
    split_node.inner_ptrs[left_size + i].setnull();
  }
  InnerPtr right_ptr = inner_fstream.allocate(right_node);

  split_node.node_size = left_size; split_node.high_kv = split_node.kv_pairs[split_node.node_size - 1];
  split_node.link_ptr = right_ptr;
  inner_fstream.write(split_node, split_ptr);

  // yeah, you can use inner_fstream.read(split_node, split_node.parent_ptr) here.
  // to reduce a copy construction.

  InnerNode parent_node; inner_fstream.read(parent_node, split_node.parent_ptr);
  ++parent_node.node_size;
  // parent_node.is_leaf = true, so no need to worry about vlist_ptrs.
  for(size_t i = parent_node.node_size - 2; i > split_pos; --i) {
    parent_node.kv_pairs[i + 1] = parent_node.kv_pairs[i];
    parent_node.inner_ptrs[i + 1] = parent_node.inner_ptrs[i];
  }
  parent_node.kv_pairs[split_pos] = split_node.high_kv;
  parent_node.kv_pairs[split_pos + 1] = right_node.high_kv;
  // assert(parent_node.inner_ptrs[split_pos] == split_ptr);
  parent_node.inner_ptrs[split_pos + 1] = right_ptr;
  inner_fstream.write(parent_node, split_node.parent_ptr);

  split_ptr = split_node.parent_ptr; split_node = parent_node;
}



} // namespace StarryPurple