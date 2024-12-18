#include "DataStructure.h"

namespace StarryPurple {

template<class KeyType, class ValueType, size_t order, size_t elementCount>
BLinkTree<KeyType, ValueType, order, elementCount>::IndexNode::IndexNode(const bool is_leaf): is_leaf_(is_leaf) {}


template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::IndexNode::InsertNonLeaf(
  const KeyType &key, const KeyPointer &key_ptr) {

}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::IndexNode::InsertLeaf(
  const KeyType &key, const KeyPointer &key_ptr, const ValueType &value, const ValuePointer &value_ptr) {

}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::IndexNode::MergeRight(const IndexPointer &index_ptr) {

}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::IndexNode::Split(const IndexPointer &index_ptr) {

}








template<class KeyType, class ValueType, size_t order, size_t elementCount>
BLinkTree<KeyType, ValueType, order, elementCount>::~BLinkTree() {
  if(is_open)
    close();
}


template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::open(
  const filenameType &filename) {
  filename_ = filename;
  index_fstream_.open(filename + "_map_index.dat");
  key_fstream_.open(filename + "_map_key.dat");
  value_fstream_.open(filename + "_map_val.dat");
  is_open = true;

  index_fstream_.read_info(root_ptr_);
}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::close() {
  index_fstream_.write_info(root_ptr_);

  index_fstream_.close();
  key_fstream_.close();
  value_fstream_.close();
  is_open = false;
}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::insert(
  const KeyType &key, const ValueType &value) {

  if(root_ptr_.isnull()) {
    // empty tree.

    IndexNode root_node{true}; // is_leaf_ = true
    root_node.node_size_ = 1;
    root_node.high_key_ = key;
    // root_node.parent_ptr_ is initially nullptr
    // root_node.link_ptr_ is initially nullptr
    root_node.key_ptr[0] = key_fstream_.allocate(key);
    // root_node.child_ptr is initially all nullptr
    root_node.value_ptr[0] = value_fstream_.allocate(value);

    root_ptr_ = index_fstream_.allocate(root_node);
    return;
  }

  // or we can do some dfs.
  IndexPointer cur_ptr = root_ptr_;
  IndexNode cur_node;
  KeyPointer helper_key;
  while(!cur_node.is_leaf_) {
    // find the leaf node
    index_fstream_.read(cur_node, cur_ptr);
    size_t l = 0, r = cur_node.node_size_ - 1;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      key_fstream_.read(helper_key, cur_node.key_ptr[mid]);
      if(key <= helper_key)
        r = mid;
      else l = mid + 1;
    }
    index_fstream_.read(cur_ptr, cur_node.child_index_ptr_[l]);
  }
  // find the place to insert.
  index_fstream_.read(cur_node, cur_ptr);
  size_t pos = 0, r = cur_node.node_size_ - 1;
  while(pos < r) {
    size_t mid = (pos + r) >> 1;
    key_fstream_.read(helper_key, cur_node.key_ptr[mid]);
    if(key <= helper_key)
      r = mid;
    else pos = mid + 1;
  }
  // insert the new node between pos - 1 and pos.
  for(size_t i = cur_node.node_size_ - 1; i >= pos; --i) {
    cur_node.key_ptr[i + 1] = cur_node.key_ptr[i];
    cur_node.value_ptr[i + 1] = cur_node.value_ptr[i];
  }
  cur_node.key_ptr[pos] = key_fstream_.allocate(key);
  cur_node.value_ptr[pos] = value_fstream_.allocate(value);
  // todo: check split.
}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
typename BLinkTree<KeyType, ValueType, order, elementCount>::ValuePointer
BLinkTree<KeyType, ValueType, order, elementCount>::find(const ValueType &value) {

}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::erase(const KeyType &key, const ValueType &value) {

}

}