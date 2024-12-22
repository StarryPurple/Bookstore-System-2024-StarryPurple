/** fmultimap.cpp
 * Author: StarryPurple
 * Date: Since 2024.12.17
 */
#include "fmultimap.h"

namespace StarryPurple {

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
Fmultimap<KeyType, ValueType, degree, elementCount>::VlistNode::VlistNode(
  const ValueType &value, const VlistPtr &next_ptr)
  :value_(value), next_ptr_(next_ptr) {}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
Fmultimap<KeyType, ValueType, degree, elementCount>::~Fmultimap() {
  if(is_open) close();
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::open(
  const filenameType &map_filename, const filenameType &vlist_filename) {
  map_fstream_.open(map_filename);
  vlist_fstream_.open(vlist_filename);
  is_open = true;

  map_fstream_.read_info(root_ptr_);
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::close() {
  map_fstream_.write_info(root_ptr_);

  map_fstream_.close();
  vlist_fstream_.close();
  is_open = false;
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
std::vector<std::pair<typename Fmultimap<KeyType, ValueType, degree, elementCount>::MnodePtr, size_t>>
Fmultimap<KeyType, ValueType, degree, elementCount>::lower_bound_route(const KeyType &key) {
  std::vector<std::pair<MnodePtr, size_t>> route;
  if(root_ptr_.isnull()) {
    return route;
  }
  MnodePtr mnode_ptr = root_ptr_;
  MapNode mnode; map_fstream_.read(mnode, mnode_ptr);
  if(key > mnode.high_key_) {
    MnodePtr cur_ptr = root_ptr_;
    MapNode cur_node; map_fstream_.read(cur_node, cur_ptr);
    while(!cur_node.is_leaf_) {
      route.emplace_back({cur_ptr, cur_node.node_size_ - 1});
      cur_ptr = cur_node.mnode_ptr_[cur_node.node_size_ - 1];
      map_fstream_.read(cur_node, cur_ptr);
    }
    route.emplace_back({cur_ptr, degree + 1});
    return route;
  }
  while(true) {
    if(key > mnode.high_key_) {
      // shouldn't reach nullptr
      mnode_ptr = mnode.link_ptr_;
      map_fstream_.read(mnode, mnode_ptr);
      continue;
    }
    size_t l = 0, r = mnode.node_size_ - 1;
    KeyType helper_key;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      helper_key = mnode.key_[mid + 1];
      if(key < helper_key) r = mid;
      else l = mid + 1;
    }
    route.emplace_back({mnode_ptr, l});
    if(mnode.is_leaf) return route;
    mnode_ptr = mnode.mnode_ptr_[l];
    map_fstream_.read(mnode, mnode_ptr);
  }
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
std::vector<std::pair<typename Fmultimap<KeyType, ValueType, degree, elementCount>::MnodePtr, size_t>>
Fmultimap<KeyType, ValueType, degree, elementCount>::upper_bound_route(const KeyType &key) {
  std::vector<std::pair<MnodePtr, size_t>> route;
  if(root_ptr_.isnull()) {
    return route;
  }
  MnodePtr mnode_ptr = root_ptr_;
  MapNode mnode; map_fstream_.read(mnode, mnode_ptr);
  if(key > mnode.high_key_) {
    MnodePtr cur_ptr = root_ptr_;
    MapNode cur_node; map_fstream_.read(cur_node, cur_ptr);
    while(!cur_node.is_leaf_) {
      route.push_back({cur_ptr, cur_node.node_size_ - 1});
      cur_ptr = cur_node.mnode_ptr_[cur_node.node_size_ - 1];
      map_fstream_.read(cur_node, cur_ptr);
    }
    route.push_back({cur_ptr, degree + 1});
    return route;
  }
  while(true) {
    if(key > mnode.high_key_) {
      // shouldn't reach nullptr
      mnode_ptr = mnode.link_ptr_;
      map_fstream_.read(mnode, mnode_ptr);
      continue;
    }
    size_t l = 0, r = mnode.node_size_ - 1;
    KeyType helper_key;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      helper_key = mnode.key_[mid];
      if(key > helper_key) l = mid + 1;
      else r = mid;
    }
    route.push_back({mnode_ptr, l});
    if(mnode.is_leaf_) return route;
    mnode_ptr = mnode.mnode_ptr_[l];
    map_fstream_.read(mnode, mnode_ptr);
  }
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::merge(const MnodePtr &parent_ptr, size_t left_pos) {
  MapNode parent_node, left_node, right_node; map_fstream_.read(parent_node, parent_ptr);
  // assert(parent_node.node_size_ > left_pos + 1);
  MnodePtr left_ptr = parent_node.mnode_ptr_[left_pos], right_ptr = parent_node.mnode_ptr_[left_pos + 1];
  map_fstream_.read(left_node, left_ptr); map_fstream_.read(right_node, right_ptr);

  const size_t offset = left_node.node_size_;
  for(size_t i = 0; i < right_node.node_size_; ++i)
    left_node.key_[offset + i] = right_node.key_[i];
  if(left_node.is_leaf_)
    for(size_t i = 0; i < right_node.node_size_; ++i)
      left_node.vlist_ptr_[offset + i] = right_node.vlist_ptr_[i];
  else
    for(size_t i = 0; i < right_node.node_size_; ++i)
      left_node.mnode_ptr_[offset + i] = right_node.mnode_ptr_[i];
  left_node.node_size_ = offset + right_node.node_size_;
  left_node.high_key_ = right_node.high_key_;
  // parent_ptr_ remains the same.
  left_node.link_ptr_ = right_node.link_ptr_;

  for(size_t i = left_pos + 1; i < parent_node.node_size_ - 1; ++i) {
    parent_node.key_[i] = parent_node.key_[i + 1];
    // always an inner node
    parent_node.mnode_ptr_[i] = parent_node.mnode_ptr_[i + 1];
  }
  --parent_node.node_size_;
  parent_node.key_[parent_node.node_size_] = KeyType();

  // parent_node modified. left_node modified. right_node discarded.
  map_fstream_.write(parent_node, parent_ptr);
  map_fstream_.write(left_node, left_ptr);
  map_fstream_.free(right_ptr);
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::move_from_left(const MnodePtr &parent_ptr, size_t left_pos) {
  MapNode parent_node, left_node, right_node; map_fstream_.read(parent_node, parent_ptr);
  // assert(parent_node.node_size_ > left_pos + 1);
  MnodePtr left_ptr = parent_node.mnode_ptr_[left_pos], right_ptr = parent_node.mnode_ptr_[left_pos + 1];
  map_fstream_.read(left_node, left_ptr); map_fstream_.read(right_node, right_ptr);

  for(size_t i = right_node.node_size_; i > 0; --i)
    right_node.key_[i] = right_node.key_[i - 1];
  right_node.key_[0] = left_node.key_[left_node.node_size_ - 1];
  if(left_node.is_leaf_) {
    for(size_t i = right_node.node_size_; i > 0; --i)
      right_node.vlist_ptr_[i] = right_node.vlist_ptr_[i - 1];
    right_node.vlist_ptr_[0] = left_node.vlist_ptr_[left_node.node_size_ - 1];
  }
  else {
    for(size_t i = right_node.node_size_; i > 0; --i)
      right_node.mnode_ptr_[i] = right_node.mnode_ptr_[i - 1];
    right_node.mnode_ptr_[0] = left_node.mnode_ptr_[left_node.node_size_ - 1];
  }
  ++right_node.node_size_;
  --left_node.node_size_;

  left_node.key_[left_node.node_size_] = KeyType();
  left_node.vlist_ptr_[left_node.node_size_].setnull();
  left_node.mnode_ptr_[left_node.node_size_].setnull();

  left_node.high_key_ = left_node.key_[left_node.node_size_ - 1];
  parent_node.key_[left_pos] = left_node.key_[left_node.node_size_ - 1];

  // parent_node modified. left_node modified. right_node modified.
  map_fstream_.write(parent_node, parent_ptr);
  map_fstream_.write(left_node, left_ptr);
  map_fstream_.write(right_node, right_ptr);
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::move_from_right(const MnodePtr &parent_ptr, size_t left_pos) {
  MapNode parent_node, left_node, right_node; map_fstream_.read(parent_node, parent_ptr);
  // assert(parent_node.node_size_ > left_pos + 1);
  MnodePtr left_ptr = parent_node.mnode_ptr_[left_pos], right_ptr = parent_node.mnode_ptr_[left_pos + 1];
  map_fstream_.read(left_node, left_ptr); map_fstream_.read(right_node, right_ptr);

  left_node.key_[left_node.node_size_] = right_node.key_[0];
  for(size_t i = 0; i < right_node.node_size_ - 1; ++i)
    left_node.key_[left_node.node_size_] = right_node.key_[0];
  if(left_node.is_leaf_) {
    left_node.vlist_ptr_[left_node.node_size_] = right_node.vlist_ptr_[0];
    for(size_t i = 0; i < right_node.node_size_ - 1; ++i)
      right_node.vlist_ptr_[i] = right_node.vlist_ptr_[i + 1];
  } else {
    left_node.mnode_ptr_[left_node.node_size_] = right_node.mnode_ptr_[0];
    for(size_t i = 0; i < right_node.node_size_ - 1; ++i)
      right_node.mnode_ptr_[i] = right_node.mnode_ptr_[i + 1];
  }
  ++left_node.node_size_;
  --right_node.node_size_;

  right_node.key_[right_node.node_size_] = KeyType();
  right_node.vlist_ptr_[right_node.node_size_].setnull();
  right_node.mnode_ptr_[right_node.node_size_].setnull();

  left_node.high_key_ = left_node.key_[left_node.node_size_ - 1];
  parent_node.key_[left_pos] = left_node.key_[left_node.node_size_ - 1];

  // parent_node modified. left_node modified. right_node modified.
  map_fstream_.write(parent_node, parent_ptr);
  map_fstream_.write(left_node, left_ptr);
  map_fstream_.write(right_node, right_ptr);
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::split(const MnodePtr &parent_ptr, size_t pos) {
  MapNode parent_node, left_node, right_node; map_fstream_.read(parent_node, parent_ptr);
  MnodePtr left_ptr = parent_node.mnode_ptr_[pos];
  map_fstream_.read(left_node, left_ptr);

  right_node.is_leaf_ = left_node.is_leaf_;
  const size_t left_size = left_node.node_size_ / 2, right_size = left_node.node_size_ - left_size;

  for(size_t i = 0; i < right_size; ++i) {
    right_node.key_[i] = left_node.key_[left_size + i];
    left_node.key_[left_size + i] = KeyType();
  }
  if(left_node.is_leaf_)
    for(size_t i = 0; i < right_size; ++i) {
      right_node.vlist_ptr_[i] = left_node.vlist_ptr_[left_size + i];
      left_node.vlist_ptr_[left_size + i].setnull();
    }
  else
    for(size_t i = 0; i < right_size; ++i) {
      right_node.mnode_ptr_[i] = left_node.mnode_ptr_[left_size + i];
      left_node.mnode_ptr_[left_size + i].setnull();
    }
  right_node.node_size_ = right_size;
  right_node.high_key_ = left_node.high_key_;
  right_node.parent_ptr_ = parent_ptr;
  right_node.link_ptr_ = left_node.link_ptr_;

  // right_node allocated.
  MnodePtr right_ptr = map_fstream_.allocate(right_node);

  left_node.node_size_ = left_size;
  left_node.high_key_ = left_node.key_[left_node.node_size_ - 1];
  left_node.link_ptr_ = right_ptr;

  // left_node modified.
  map_fstream_.write(left_node, left_ptr);

  for(size_t i = parent_node.node_size_ - 1; i > pos; --i) {
    parent_node.key_[i + 1] = parent_node.key_[i];
    // always an inner node
    parent_node.mnode_ptr_[i + 1] = parent_node.mnode_ptr_[i];
  }
  parent_node.key_[pos + 1] = parent_node.key_[pos];
  parent_node.key_[pos] = left_node.key_[left_node.node_size_ - 1];
  parent_node.mnode_ptr_[pos + 1] = right_ptr;
  ++parent_node.node_size_;

  // parent_node modified.
  map_fstream_.write(parent_node, parent_ptr);
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::insert(const KeyType &key, const ValueType &value) {
  if(root_ptr_.isnull()) {
    // empty tree.
    // just create a root node.

    MapNode root_node;
    root_node.is_leaf_ = true;
    root_node.node_size_ = 1;
    root_node.high_key_ = key;
    root_node.parent_ptr_.setnull();
    root_node.link_ptr_.setnull();
    root_node.key_[0] = key;
    root_node.vlist_ptr_[0] = vlist_fstream_.allocate(VlistNode(value, VlistPtr(nullptr)));

    root_ptr_ = map_fstream_.allocate(root_node);
    return;
  }
  std::vector<std::pair<MnodePtr, size_t>> route = upper_bound_route(key);
  // route can't be empty, for tree can't be empty.
  auto [leaf_ptr, pos] = route.back();
  route.pop_back();
  MnodePtr cur_ptr = leaf_ptr;
  MapNode cur_node; map_fstream_.read(cur_node, cur_ptr);
  if(pos == degree + 1) {
    // This key doesn't exist, and it's bigger than any existing key
    cur_node.key_[cur_node.node_size_] = key;
    cur_node.vlist_ptr_[cur_node.node_size_] = vlist_fstream_.allocate(VlistNode(value, VlistPtr(nullptr)));
    cur_node.high_key_ = key;
    ++cur_node.node_size_;
    map_fstream_.write(cur_node, cur_ptr);
    for(auto [route_ptr, pos]: route) {
      MapNode route_node;
      map_fstream_.read(route_node, route_ptr);
      route_node.key_[route_node.node_size_ - 1] = key; // pos == route_node.node_size_
      route_node.high_key_ = key;
      map_fstream_.write(route_node, route_ptr);
    }
  } else {
    KeyType helper_key;
    helper_key = cur_node.key_[pos];
    if(key != helper_key) {
      // This key doesn't exist, but there is a key bigger than it.
      for(size_t i = cur_node.node_size_; i > pos; --i) {
        cur_node.key_[i] = cur_node.key_[i - 1];
        cur_node.vlist_ptr_[i] = cur_node.vlist_ptr_[i - 1];
      }
      cur_node.key_[pos] = key;
      cur_node.vlist_ptr_[pos] = vlist_fstream_.allocate(VlistNode(value, VlistPtr(nullptr)));
      ++cur_node.node_size_;
      map_fstream_.write(cur_node, cur_ptr);
    } else {
      // This key already exists.
      VlistPtr cur_vlist_ptr = cur_node.vlist_ptr_[pos];
      if(cur_vlist_ptr.isnull()) {
        // should not happen.
        // throw BLinkTreeExceptions("Empty key still exists");
        cur_node.vlist_ptr_[pos] = vlist_fstream_.allocate(VlistNode(value, VlistPtr(nullptr)));
        map_fstream_.write(cur_node, cur_ptr);
      } else {
        VlistNode cur_vlist_node; vlist_fstream_.read(cur_vlist_node, cur_vlist_ptr);
        if(value == cur_vlist_node.value_){
          // throw BLinkTreeExceptions("Inserting duplicated key-value pair");
          return;
        }
        if(value < cur_vlist_node.value_) {
          VlistPtr new_vlist_ptr = vlist_fstream_.allocate(VlistNode(value, cur_vlist_ptr));
          cur_node.vlist_ptr_[pos] = new_vlist_ptr;
          map_fstream_.write(cur_node, cur_ptr);
        } else {
          VlistPtr nxt_vlist_ptr = cur_vlist_node.next_ptr_;
          VlistNode nxt_vlist_node;
          while(true) {
            if(nxt_vlist_ptr.isnull()) {
              cur_vlist_node.next_ptr_ = vlist_fstream_.allocate(VlistNode(value, VlistPtr(nullptr)));
              vlist_fstream_.write(cur_vlist_node, cur_vlist_ptr);
              break;
            }
            vlist_fstream_.read(nxt_vlist_node, nxt_vlist_ptr);
            if(value == nxt_vlist_node.value_) {
              // throw BLinkTreeExceptions("Inserting duplicated key-value pair");
              return;
            }
            if(value < nxt_vlist_node.value_) {
              cur_vlist_node.next_ptr_ = vlist_fstream_.allocate(VlistNode(value, nxt_vlist_ptr));
              vlist_fstream_.write(cur_vlist_node, cur_vlist_ptr);
              break;
            }
            cur_vlist_ptr = nxt_vlist_ptr;
            cur_vlist_node = nxt_vlist_node;
            nxt_vlist_ptr = cur_vlist_node.next_ptr_;
          }
        }
      }
    }
  }

  // split.
  while(cur_node.node_size_ == degree) {
    // MapNode split is required.
    if(route.empty()) {
      // cur_node = root_node_ now.
      // create a new root node.
      // map_fstream.read(root_node, root_ptr_); assert(root_node == cur_node);
      MapNode root_node;
      MnodePtr cur_root_ptr = root_ptr_;
      root_node.is_leaf_ = false; // never a leaf: the tree isn't empty
      root_node.node_size_ = 1;
      root_node.high_key_ = cur_node.high_key_;
      root_node.key_[0] = cur_node.key_[cur_node.node_size_ - 1];
      root_node.mnode_ptr_[0] = cur_root_ptr;
      root_ptr_ = map_fstream_.allocate(root_node);
      cur_node.parent_ptr_ = root_ptr_; map_fstream_.write(cur_node, cur_root_ptr);
      split(root_ptr_, 0);
      return;
    }
    auto [parent_ptr, pos] = route.back();
    route.pop_back();
    split(parent_ptr, pos);
    map_fstream_.read(cur_node, parent_ptr);
  }
}


template<class KeyType, class ValueType, size_t degree, size_t elementCount>
std::vector<ValueType> Fmultimap<KeyType, ValueType, degree, elementCount>::find(const KeyType &key) {
  std::vector<ValueType> res;
  std::vector<std::pair<MnodePtr, size_t>> route = upper_bound_route(key);
  if(route.empty()) return res; // nothing found.
  auto [leaf_ptr, pos] = route.back();
  if(pos == degree + 1) return res; // nothing found.
  MapNode map_node; map_fstream_.read(map_node, leaf_ptr);
  KeyType key_found; key_found = map_node.key_[pos]; // combine these lines.
  if(key != key_found) return res; // not found. maybe "key < key_found"?
  VlistPtr cur_vlist_ptr = map_node.vlist_ptr_[pos];
  VlistNode cur_vlist_node;
  while(!cur_vlist_ptr.isnull()) {
    vlist_fstream_.read(cur_vlist_node, cur_vlist_ptr);
    res.emplace_back(cur_vlist_node.value_);
    cur_vlist_ptr = cur_vlist_node.next_ptr_;
  }
  return res;
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void Fmultimap<KeyType, ValueType, degree, elementCount>::erase(const KeyType &key, const ValueType &value) {
  if(root_ptr_.isnull()) return; // nothing to delete.
  std::vector<std::pair<MnodePtr, size_t>> route = upper_bound_route(key);
  // route can't be empty, for tree is not empty
  auto [leaf_ptr, pos] = route.back();
  route.pop_back();
  if(pos == degree + 1) return; // nothing to delete.
  MnodePtr cur_ptr = leaf_ptr;
  MapNode cur_node; map_fstream_.read(cur_node, cur_ptr);
  VlistPtr cur_vlist_ptr = cur_node.vlist_ptr_[pos];
  if(cur_vlist_ptr.isnull()) {
    // shouldn't have been here.
    // throw BLinkTreeExceptions("Empty key still exists.");
    return; // nothing to delete
  }
  VlistNode cur_vlist_node; vlist_fstream_.read(cur_vlist_node, cur_vlist_ptr);
  VlistPtr nxt_vlist_ptr = cur_vlist_node.next_ptr_;
  if(nxt_vlist_ptr.isnull()) {
    if(value == cur_vlist_node.value_) {
      vlist_fstream_.free(cur_vlist_ptr);
      // cur_node.key_[pos].setnull();
      // map_fstream_.write(cur_node, cur_ptr);
      // delete this key.
      for(size_t i = pos; i < cur_node.node_size_ - 1; ++i) {
        // leaf_node
        cur_node.key_[i] = cur_node.key_[i + 1];
        cur_node.vlist_ptr_[i] = cur_node.vlist_ptr_[i + 1];
      }
      --cur_node.node_size_;
      cur_node.key_[cur_node.node_size_] = KeyType();
      cur_node.vlist_ptr_[cur_node.node_size_].setnull();
      if(cur_ptr == root_ptr_) {
        // only check if the tree is empty
        if(cur_node.node_size_ == 0) {
          map_fstream_.free(cur_ptr);
          root_ptr_.setnull();
          return;
        }
      }
      // now cur_node.node_size >= degree / 2 - 1 >= 2. (with degree >= 6)
      if(pos == cur_node.node_size_)
        cur_node.high_key_ = cur_node.key_[cur_node.node_size_ - 1];
      map_fstream_.write(cur_node, cur_ptr);
      while(true) {
        // handle with MapNodes that is too small.
        if(cur_node.node_size_ >= degree / 2) return;
        if(route.empty()) {
          // we accept a root node with 1 ~ degree / 2 - 1 nodes.
          return;
        }
        auto [parent_ptr, pos] = route.back();
        route.pop_back();
        MapNode parent_node; map_fstream_.read(parent_node, parent_ptr);
        if(pos > 0) {
          // use left.
          MapNode left_node; map_fstream_.read(left_node, parent_node.mnode_ptr_[pos - 1]);
          if(left_node.node_size_ >= degree / 2) {
            move_from_left(parent_ptr, pos);
            return;
          } else {
            merge(parent_ptr, pos - 1);
            // try modified parent_node
            map_fstream_.read(cur_node, parent_ptr);
            continue;
          }
        }
        if(pos < cur_node.node_size_ - 1) {
          // try right.
          MapNode right_node; map_fstream_.read(right_node, parent_node.mnode_ptr_[pos + 1]);
          if(right_node.node_size_ >= degree / 2) {
            move_from_right(parent_ptr, pos);
            return;
          } else {
            merge(parent_ptr, pos);
            // try modified parent_node
            map_fstream_.read(cur_node, parent_ptr);
            continue;
          }
        }
        throw BLinkTreeExceptions("inner node size < 2");
      }
    }
    return;
  }
  if(value == cur_vlist_node.value_) {
    cur_node.vlist_ptr_[pos] = nxt_vlist_ptr;
    // cur_node modified.
    map_fstream_.write(cur_node, cur_ptr);
    vlist_fstream_.free(cur_vlist_ptr);
    return;
  }
  VlistNode nxt_vlist_node;
  while(!nxt_vlist_ptr.isnull()) {
    vlist_fstream_.read(nxt_vlist_node, nxt_vlist_ptr);
    if(value == nxt_vlist_node.value_) {
      cur_vlist_node.next_ptr_ = nxt_vlist_node.next_ptr_;
      // cur_vlist_node modified.
      vlist_fstream_.write(cur_vlist_node, cur_vlist_ptr);
      vlist_fstream_.free(nxt_vlist_ptr);
      return;
    }
    cur_vlist_ptr = nxt_vlist_ptr;
    cur_vlist_node = nxt_vlist_node;
    nxt_vlist_ptr = cur_vlist_node.next_ptr_;
  }
  // this key-value doesn't exist.
}


} // namespace StarryPurple