#include "fmultimap.h"

namespace StarryPurple {

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
Fmultimap<KeyType, ValueType, degree, elementCount>::~Fmultimap() {
  if(is_open)
    close();
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::open(
  const filenameType &name_suffix) {
  inner_fstream.open(name_suffix + "_inner.bsdat");
  vlist_fstream.open(name_suffix + "_vlist.bsdat");
  is_open = true;
  inner_fstream.read_info(root_ptr);
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::close() {
  inner_fstream.write_info(root_ptr);
  inner_fstream.close();
  vlist_fstream.close();
  is_open = false;
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::insert(
  const KeyType &key, const ValueType &value) {
  if(root_ptr.isnull()) {
    VlistNode vlist_node;
    vlist_node.value = value; vlist_node.nxt.setnull();
    VlistNode vlist_begin_node;
    vlist_begin_node.nxt = vlist_fstream.allocate(vlist_node);

    InnerNode inner_root_node;
    inner_root_node.parent_ptr.setnull();
    inner_root_node.is_leaf = true; inner_root_node.high_key = key; inner_root_node.node_size = 1;
    inner_root_node.keys[0] = key; inner_root_node.vlist_ptrs[0] = vlist_fstream.allocate(vlist_begin_node);
    root_ptr = inner_fstream.allocate(inner_root_node);
    // initialize requires no maintain_size.
    return;
  }
  InnerPtr cur_inner_ptr = root_ptr;
  InnerNode cur_inner_node; inner_fstream.read(cur_inner_node, cur_inner_ptr);
  size_t pos;
  std::vector<size_t> pos_route; // records the pos every time we choose.
  while(true) {
    if(cur_inner_node.is_leaf && key > cur_inner_node.high_key) {
      VlistNode vlist_node;
      vlist_node.value = value; vlist_node.nxt.setnull();
      VlistNode vlist_begin_node;
      vlist_begin_node.nxt = vlist_fstream.allocate(vlist_node);

      ++cur_inner_node.node_size;
      cur_inner_node.high_key = key;
      cur_inner_node.keys[cur_inner_node.node_size - 1] = key;
      cur_inner_node.vlist_ptrs[cur_inner_node.node_size - 1] = vlist_fstream.allocate(vlist_begin_node);
      inner_fstream.write(cur_inner_node, cur_inner_ptr);
      // node_size modified. Start maintenance.
      maintain_size(pos_route, cur_inner_ptr, cur_inner_node);
      return;
    }
    if(cur_inner_node.is_leaf) {
      size_t l = 0, r = cur_inner_node.node_size - 1;
      while(l < r) {
        size_t mid = (l + r) >> 1;
        if(key > cur_inner_node.keys[mid]) l = mid + 1;
        else r = mid;
      }
      pos = l;
      break;
    }
    if(key > cur_inner_node.high_key) {
      pos_route.push_back(cur_inner_node.node_size - 1);
      cur_inner_node.high_key = key;
      cur_inner_node.keys[cur_inner_node.node_size - 1] = key;
      inner_fstream.write(cur_inner_node, cur_inner_ptr);
      cur_inner_ptr = cur_inner_node.inner_ptrs[cur_inner_node.node_size - 1];
      inner_fstream.read(cur_inner_node, cur_inner_ptr);
      continue;
    }
    size_t l = 0, r = cur_inner_node.node_size - 1;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      if(key > cur_inner_node.keys[mid]) l = mid + 1;
      else r = mid;
    }
    pos_route.push_back(l);
    cur_inner_ptr = cur_inner_node.inner_ptrs[l];
    inner_fstream.read(cur_inner_node, cur_inner_ptr);
  }
  if(key < cur_inner_node.keys[pos]) {
    VlistNode vlist_node;
    vlist_node.value = value; vlist_node.nxt.setnull();
    VlistNode vlist_begin_node;
    vlist_begin_node.nxt = vlist_fstream.allocate(vlist_node);

    ++cur_inner_node.node_size;
    for(size_t i = cur_inner_node.node_size - 1; i > pos; --i) {
      cur_inner_node.keys[i] = cur_inner_node.keys[i - 1];
      cur_inner_node.vlist_ptrs[i] = cur_inner_node.vlist_ptrs[i - 1];
    }
    cur_inner_node.keys[pos] = key;
    cur_inner_node.vlist_ptrs[pos] = vlist_fstream.allocate(vlist_begin_node);
    inner_fstream.write(cur_inner_node, cur_inner_ptr);
    // node_size modified. Start maintenance.
    maintain_size(pos_route, cur_inner_ptr, cur_inner_node);
    return;
  }
  // assert(key == cur_inner_node.keys[pos]);
  VlistPtr cur_vlist_ptr = cur_inner_node.vlist_ptrs[pos];
  VlistNode cur_vlist_node; vlist_fstream.read(cur_vlist_node, cur_vlist_ptr);
  VlistPtr nxt_vlist_ptr = cur_vlist_node.nxt;
  VlistNode nxt_vlist_node;
  while(!nxt_vlist_ptr.isnull()) {
    vlist_fstream.read(nxt_vlist_node, nxt_vlist_ptr);
    if(value == nxt_vlist_node.value) return; // ignore inserting duplicated key-value pair
    if(value < nxt_vlist_node.value) {
      VlistNode new_vlist_node;
      new_vlist_node.value = value; new_vlist_node.nxt = nxt_vlist_ptr;
      cur_vlist_node.nxt = vlist_fstream.allocate(new_vlist_node);
      vlist_fstream.write(cur_vlist_node, cur_vlist_ptr);
      return;
    }
    cur_vlist_node = nxt_vlist_node;
    cur_vlist_ptr = nxt_vlist_ptr;
    nxt_vlist_ptr = cur_vlist_node.nxt;
  }
  VlistNode new_vlist_node;
  new_vlist_node.value = value; // new_vlist_node.nxt = nxt_vlist_ptr = "nullptr"
  cur_vlist_node.nxt = vlist_fstream.allocate(new_vlist_node);
  vlist_fstream.write(cur_vlist_node, cur_vlist_ptr);
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::erase(
  const KeyType &key, const ValueType &value) {
  if(root_ptr.isnull()) return;
  InnerPtr cur_inner_ptr = root_ptr;
  InnerNode cur_inner_node; inner_fstream.read(cur_inner_node, cur_inner_ptr);
  if(key > cur_inner_node.high_key) return; // key too large
  size_t pos;
  while(true) {
    size_t l = 0, r = cur_inner_node.node_size - 1;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      if(key > cur_inner_node.keys[mid]) l = mid + 1;
      else r = mid;
    }
    if(cur_inner_node.is_leaf) {
      pos = l;
      break;
    }
    cur_inner_ptr = cur_inner_node.inner_ptrs[l];
    inner_fstream.read(cur_inner_node, cur_inner_ptr);
  }
  if(key < cur_inner_node.keys[pos]) return; // key not exist
  // assert(key == cur_inner_node.keys[pos])
  VlistPtr cur_vlist_ptr = cur_inner_node.vlist_ptrs[pos];
  VlistNode cur_vlist_node; vlist_fstream.read(cur_vlist_node, cur_vlist_ptr);
  VlistPtr nxt_vlist_ptr = cur_vlist_node.nxt;
  VlistNode nxt_vlist_node;
  while(!nxt_vlist_ptr.isnull()) {
    vlist_fstream.read(nxt_vlist_node, nxt_vlist_ptr);
    if(value == nxt_vlist_node.value) {
      cur_vlist_node.nxt = nxt_vlist_node.nxt;
      vlist_fstream.write(cur_vlist_node, cur_vlist_ptr);
      vlist_fstream.free(nxt_vlist_ptr);
      /* In order to avoid merging / averaging operations and their time complexty cost,
       * Here we choose not to discard this key,
       * at the cost of more space occupied (commonly doesn't matter) and B-plus Tree (for now) layers.
       * But, if degree is big enough, the latter drawback will also be tiny.
      if(cur_vlist_ptr == cur_inner_node.vlist_ptrs[pos] && cur_vlist_node.nxt.isnull()) {
        // discard this key.
        ...
        // maintain_size for cur_inner_node with a size shrink
        maintain_size(..., ..., ..., cur_inner_node);
      }
      */
      return;
    }
    if(value < nxt_vlist_node.value) return; // value not exist
    cur_vlist_node = nxt_vlist_node;
    cur_vlist_ptr = nxt_vlist_ptr;
    nxt_vlist_ptr = cur_vlist_node.nxt;
  }
  // if code reaches here, it means: value too big.
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
std::vector<ValueType> Fmultimap<KeyType, ValueType, degree, elementCount>::operator[](
  const KeyType &key) {
  std::vector<ValueType> res;
  if(root_ptr.isnull()) return res;
  InnerPtr cur_inner_ptr = root_ptr;
  InnerNode cur_inner_node; inner_fstream.read(cur_inner_node, cur_inner_ptr);
  if(key > cur_inner_node.high_key) return res; // key too large
  size_t pos;
  while(true) {
    size_t l = 0, r = cur_inner_node.node_size - 1;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      if(key > cur_inner_node.keys[mid]) l = mid + 1;
      else r = mid;
    }
    if(cur_inner_node.is_leaf) {
      pos = l;
      break;
    }
    cur_inner_ptr = cur_inner_node.inner_ptrs[l];
    inner_fstream.read(cur_inner_node, cur_inner_ptr);
  }
  if(key < cur_inner_node.keys[pos]) return res; // key not exist
  // assert(key == cur_inner_node.keys[pos])
  VlistPtr cur_vlist_ptr = cur_inner_node.vlist_ptrs[pos];
  VlistNode cur_vlist_node; vlist_fstream.read(cur_vlist_node, cur_vlist_ptr);
  VlistPtr nxt_vlist_ptr = cur_vlist_node.nxt;
  VlistNode nxt_vlist_node;
  while(!nxt_vlist_ptr.isnull()) {
    vlist_fstream.read(nxt_vlist_node, nxt_vlist_ptr);
    res.push_back(nxt_vlist_node.value);
    cur_vlist_node = nxt_vlist_node;
    cur_vlist_ptr = nxt_vlist_ptr;
    nxt_vlist_ptr = cur_vlist_node.nxt;
  }
  return res;
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::maintain_size(
  std::vector<size_t> &pos_route, InnerPtr &maintain_ptr, InnerNode &maintain_node) {
  // Temporarily we just deal with node_size large enough to split.
  while(!pos_route.empty() && maintain_node.node_size >= degree) {
    size_t pos = pos_route.back();
    pos_route.pop_back();
    split(pos, maintain_ptr, maintain_node);
  }
  // here maintain_ptr = root_ptr, maintain_node is the root node.
  // assert(maintain_ptr == root_ptr);
  if(maintain_node.node_size >= degree) {
    // create a new root.
    InnerNode new_root;
    new_root.is_leaf = false; new_root.high_key = maintain_node.high_key;
    // new_root.parent_ptr = "nullptr"
    new_root.node_size = 1;
    new_root.keys[0] = maintain_node.high_key;
    new_root.inner_ptrs[0] = maintain_ptr;
    // You can do the split completely here to reduce two file operations.
    // But I'm lazy now. or to say crazy? So I refuse.
    root_ptr = maintain_node.parent_ptr = inner_fstream.allocate(new_root);
    inner_fstream.write(maintain_node, maintain_ptr);
    split(0, maintain_ptr, maintain_node);
  }
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::split(
  const size_t &split_pos, InnerPtr &split_ptr, InnerNode &split_node) {
  // split a new node and insert it into the right pos.
  size_t left_size = split_node.node_size / 2, right_size = split_node.node_size - left_size;

  InnerNode right_node;
  right_node.is_leaf = split_node.is_leaf; right_node.high_key = split_node.high_key;
  right_node.parent_ptr = split_node.parent_ptr; right_node.node_size = right_size;
  KeyType empty_key;
  for(size_t i = 0; i < right_size; ++i) {
    right_node.keys[i] = split_node.keys[left_size + i];
    right_node.inner_ptrs[i] = split_node.inner_ptrs[left_size + i];
    right_node.vlist_ptrs[i] = split_node.vlist_ptrs[left_size + i];
    split_node.keys[left_size + i] = empty_key;
    split_node.inner_ptrs[left_size + i].setnull();
    split_node.vlist_ptrs[left_size + i].setnull();
  }
  InnerPtr right_ptr = inner_fstream.allocate(right_node);

  split_node.node_size = left_size; split_node.high_key = split_node.keys[split_node.node_size - 1];
  inner_fstream.write(split_node, split_ptr);

  // yeah, you can use inner_fstream.read(split_node, split_node.parent_ptr) here.
  // to reduce a copy construction.

  InnerNode parent_node; inner_fstream.read(parent_node, split_node.parent_ptr);
  ++parent_node.node_size;
  // parent_node.is_leaf = true, so no need to worry about vlist_ptrs.
  for(size_t i = parent_node.node_size - 2; i > split_pos; --i) {
    parent_node.keys[i + 1] = parent_node.keys[i];
    parent_node.inner_ptrs[i + 1] = parent_node.inner_ptrs[i];
  }
  parent_node.keys[split_pos] = split_node.high_key;
  parent_node.keys[split_pos + 1] = right_node.high_key;
  // assert(parent_node.inner_ptrs[split_pos] == split_ptr)
  parent_node.inner_ptrs[split_pos + 1] = right_ptr;
  inner_fstream.write(parent_node, split_node.parent_ptr);

  split_ptr = split_node.parent_ptr; split_node = parent_node;
}



} // namespace StarryPurple