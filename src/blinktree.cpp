#include "blinktree.h"

namespace StarryPurple {

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
BLinkTree<KeyType, ValueType, degree, elementCount>::~BLinkTree() {
  if(is_open) close();
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void BLinkTree<KeyType, ValueType, degree, elementCount>::open(const filenameType &map_filename, const filenameType &key_filename, const filenameType &value_filename) {
  map_fstream_.open(map_filename);
  key_fstream_.open(key_filename);
  value_fstream_.open(value_filename);
  is_open = true;

  map_fstream_.read_info(root_ptr_);
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void BLinkTree<KeyType, ValueType, degree, elementCount>::close() {
  map_fstream_.write_info(root_ptr_);

  map_fstream_.close();
  key_fstream_.close();
  value_fstream_.close();
  is_open = false;
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
std::pair<typename BLinkTree<KeyType, ValueType, degree, elementCount>::MnodePtr, size_t>
BLinkTree<KeyType, ValueType, degree, elementCount>::lower_bound(const KeyType &key) {
  if(root_ptr_.isnull()) return {nullptr, 0};
  MnodePtr mnode_ptr = root_ptr_;
  MapNode mnode; map_fstream_.read(mnode, mnode_ptr);
  while(true) {
    if(key > mnode.high_key_) {
      mnode_ptr = mnode.link_ptr_;
      if(mnode_ptr.isnull()) return {nullptr, 0};
      map_fstream_.read(mnode, mnode_ptr);
      continue;
    }
    size_t l = 0, r = mnode.node_size_ - 1;
    KeyType helper_key;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      key_fstream_.read(helper_key, mnode.key_ptr_[mid + 1]);
      if(key < helper_key) r = mid;
      else l = mid + 1;
    }
    if(mnode.is_leaf) return {mnode_ptr, l};
    mnode_ptr = mnode.mnode_ptr_[l];
    map_fstream_.read(mnode, mnode_ptr);
  }
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
std::pair<typename BLinkTree<KeyType, ValueType, degree, elementCount>::MnodePtr, size_t>
BLinkTree<KeyType, ValueType, degree, elementCount>::upper_bound(const KeyType &key) {
  if(root_ptr_.isnull()) return {nullptr, 0};
  MnodePtr mnode_ptr = root_ptr_;
  MapNode mnode; map_fstream_.read(mnode, mnode_ptr);
  while(true) {
    if(key > mnode.high_key_) {
      mnode_ptr = mnode.link_ptr_;
      if(mnode_ptr.isnull()) return {nullptr, 0};
      map_fstream_.read(mnode, mnode_ptr);
      continue;
    }
    size_t l = 0, r = mnode.node_size_ - 1;
    KeyType helper_key;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      key_fstream_.read(helper_key, mnode.key_ptr_[mid]);
      if(key > helper_key) l = mid + 1;
      else r = mid;
    }
    if(mnode.is_leaf) return {mnode_ptr, l};
    mnode_ptr = mnode.mnode_ptr_[l];
    map_fstream_.read(mnode, mnode_ptr);
  }
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void BLinkTree<KeyType, ValueType, degree, elementCount>::merge(const MnodePtr &parent_ptr, size_t left_pos) {
  MapNode parent_node, left_node, right_node; map_fstream_.read(parent_node, parent_ptr);
  // assert(parent_node.node_size_ > left_pos + 1);
  MnodePtr left_ptr = parent_node.mnode_ptr_[left_pos], right_ptr = parent_node.mnode_ptr_[left_pos + 1];
  map_fstream_.read(left_node, left_ptr); map_fstream_.read(right_node, right_ptr);

  const size_t offset = left_node.node_size_;
  for(size_t i = 0; i < right_node.node_size_; ++i)
    left_node.key_ptr[offset + i] = right_node.key_ptr_[i];
  if(left_node.is_leaf_)
    for(size_t i = 0; i < right_node.node_size_; ++i)
      left_node.vlist_ptr_[offset + i] = right_node.vlist_ptr[i];
  else
    for(size_t i = 0; i < right_node.node_size_; ++i)
      left_node.mnode_ptr_[offset + i] = right_node.mnode_ptr_[i];
  left_node.node_size_ = offset + right_node.node_size_;
  left_node.high_key_ = right_node.high_key_;
  // parent_ptr_ remains the same.
  left_node.link_ptr_ = right_node.link_ptr_;

  for(size_t i = left_pos + 1; i < parent_node.node_size_ - 1; ++i) {
    parent_node.key_ptr_[i] = parent_node.key_ptr_[i + 1];
    // always an inner node
    parent_node.mnode_ptr_[i] = parent_node.mnode_ptr_[i + 1];
  }
  --parent_node.node_size_;
  parent_node.key_ptr_[parent_node.node_size_].setnull();

  // parent_node modified. left_node modified. right_node discarded.
  map_fstream_.write(parent_node, parent_ptr);
  map_fstream_.write(left_node, left_ptr);
  map_fstream_.free(right_node, right_ptr);
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void BLinkTree<KeyType, ValueType, degree, elementCount>::move_from_left(const MnodePtr &parent_ptr, size_t left_pos) {
  MapNode parent_node, left_node, right_node; map_fstream_.read(parent_node, parent_ptr);
  // assert(parent_node.node_size_ > left_pos + 1);
  MnodePtr left_ptr = parent_node.mnode_ptr_[left_pos], right_ptr = parent_node.mnode_ptr_[left_pos + 1];
  map_fstream_.read(left_node, left_ptr); map_fstream_.read(right_node, right_ptr);

  for(size_t i = right_node.node_size_; i > 0; --i)
    right_node.key_ptr_[i] = right_node.key_ptr_[i - 1];
  right_node.key_ptr_[0] = left_node.key_ptr_[left_node.node_size_ - 1];
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

  left_node.key_ptr_[left_node.node_size_].setnull();
  left_node.vlist_ptr_[left_node.node_size_].setnull();
  left_node.mnode_ptr_[left_node.node_size_].setnull();

  key_fstream_.read(left_node.high_key_, left_node.key_ptr_[left_node.node_size_ - 1]);
  parent_node.key_ptr_[left_pos] = left_node.key_ptr_[left_node.node_size_ - 1];

  // parent_node modified. left_node modified. right_node modified.
  map_fstream_.write(parent_node, parent_ptr);
  map_fstream_.write(left_node, left_ptr);
  map_fstream_.write(right_node, right_ptr);
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void BLinkTree<KeyType, ValueType, degree, elementCount>::move_from_right(const MnodePtr &parent_ptr, size_t left_pos) {
  MapNode parent_node, left_node, right_node; map_fstream_.read(parent_node, parent_ptr);
  // assert(parent_node.node_size_ > left_pos + 1);
  MnodePtr left_ptr = parent_node.mnode_ptr_[left_pos], right_ptr = parent_node.mnode_ptr_[left_pos + 1];
  map_fstream_.read(left_node, left_ptr); map_fstream_.read(right_node, right_ptr);

  left_node.key_ptr_[left_node.node_size_] = right_node.key_ptr_[0];
  for(size_t i = 0; i < right_node.node_size_ - 1; ++i)
    left_node.key_ptr_[left_node.node_size_] = right_node.key_ptr_[0];
  if(left_node.is_leaf_) {
    left_node.vlist_ptr_[left_node.node_size_] = right_node.vlist_ptr_[0];
    for(size_t i = 0; i < right_node.node_size_ - 1; ++i)
      right_node.vlist_ptr_[i] = right_node.vlist_ptr_[i + 1];
  } else {
    left_node.mnode_ptr_[left_node.node_size_] = right_node.vlist_ptr_[0];
    for(size_t i = 0; i < right_node.node_size_ - 1; ++i)
      right_node.mnode_ptr_[i] = right_node.mnode_ptr_[i + 1];
  }
  ++left_node.node_size_;
  --right_node.node_size_;

  right_node.key_ptr_[right_node.node_size_].setnull();
  right_node.vlist_ptr_[right_node.node_size_].setnull();
  right_node.mnode_ptr_[right_node.node_size_].setnull();

  key_fstream_.read(left_node.high_key_, left_node.key_ptr_[left_node.node_size_ - 1]);
  parent_node.key_ptr_[left_pos] = left_node.key_ptr_[left_node.node_size_ - 1];

  // parent_node modified. left_node modified. right_node modified.
  map_fstream_.write(parent_node, parent_ptr);
  map_fstream_.write(left_node, left_ptr);
  map_fstream_.write(right_node, right_ptr);
}

template<class KeyType, class ValueType, size_t degree, size_t elementCount>
void BLinkTree<KeyType, ValueType, degree, elementCount>::split(const MnodePtr &parent_ptr, size_t pos) {
  MapNode parent_node, left_node, right_node; map_fstream_.read(parent_node, parent_ptr);
  MnodePtr left_ptr = parent_node.mnode_ptr_[pos];
  map_fstream_.read(left_node, left_ptr);

  right_node.is_leaf_ = left_node.is_leaf_;
  const size_t left_size = left_node.node_size_ / 2, right_size = left_node.node_size_ - left_size;

  for(size_t i = 0; i < right_size; ++i) {
    right_node.key_ptr_[i] = left_node.key_ptr_[left_size + i];
    left_node.key_ptr_[left_size + i].setnull();
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
  key_fstream_.read(left_node.high_key_, left_node.key_ptr_[left_node.node_size_ - 1]);
  left_node.link_ptr_ = right_ptr;

  // left_node modified.
  map_fstream_.write(left_node, left_ptr);

  for(size_t i = parent_node.node_size_ - 1; i > pos; --i) {
    parent_node.key_ptr_[i + 1] = parent_node.key_ptr_[i];
    // always an inner node
    parent_node.mnode_ptr_[i + 1] = parent_node.mnode_ptr_[i];
  }
  parent_node.key_ptr_[pos + 1] = parent_node.key_ptr_[pos];
  parent_node.key_ptr_[pos] = left_node.key_ptr_[left_node.node_size_ - 1];
  parent_node.mnode_ptr_[pos + 1] = right_ptr;

  // parent_node modified.
  map_fstream_.write(parent_node, parent_ptr);
}







}