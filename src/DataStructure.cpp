#include "DataStructure.h"

namespace StarryPurple {

template<class KeyType, class ValueType, size_t order, size_t elementCount>
BLinkTree<KeyType, ValueType, order, elementCount>::IndexNode::IndexNode(const bool is_leaf): is_leaf_(is_leaf) {}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::Split(const IndexPointer &index_ptr) {
  assert(false, "Under development");
}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::MergeLeft(const IndexPointer &index_ptr) {
  assert(false, "Under development");
}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::MergeRight(const IndexPointer &index_ptr) {
  assert(false, "Under development");
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
  vlist_fstream_.open(filename + "_map_vlist.dat");
  is_open = true;

  InfoType info;
  index_fstream_.read_info(info);
  root_ptr_ = info.root_ptr_;
  end_ptr_ = info.end_ptr;
}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::close() {
  InfoType info;
  info.root_ptr_ = root_ptr_;
  info.end_ptr = end_ptr_;
  index_fstream_.write_info(info);

  index_fstream_.close();
  key_fstream_.close();
  vlist_fstream_.close();
  is_open = false;
}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
BLinkTree<KeyType, ValueType, order, elementCount>::ValueListNode::ValueListNode(const ValueType &value) {
  value_ = value;
}


template<class KeyType, class ValueType, size_t order, size_t elementCount>
std::pair<typename BLinkTree<KeyType, ValueType, order, elementCount>::IndexPointer, size_t>
BLinkTree<KeyType, ValueType, order, elementCount>::upper_bound(const KeyType &key) {
  if(root_ptr_.isnull())
    return {nullptr, 0};
  IndexPointer cur_ptr = root_ptr_;
  IndexNode cur_node;
  index_fstream_.read(cur_node, root_ptr_);
  KeyPointer helper_key;
  while(!cur_node.is_leaf_) {
    // find the leaf node
    if(key > cur_node.high_key_) {
      cur_ptr = cur_node.link_ptr_;
      if(cur_ptr.isnull())
        return {nullptr, 0}; // key is bigger than all key_s
      index_fstream_.read(cur_node, cur_ptr);
      continue;
    }
    size_t l = 0, r = cur_node.node_size_ - 1;
    while(l < r) {
      size_t mid = (l + r) >> 1;
      key_fstream_.read(helper_key, cur_node.key_ptr[mid]);
      if(key <= helper_key)
        r = mid;
      else l = mid + 1;
    }
    index_fstream_.read(cur_ptr, cur_node.child_index_ptr_[l]);
    index_fstream_.read(cur_node, cur_ptr);
  }
  // upper_bound.
  index_fstream_.read(cur_node, cur_ptr);
  size_t pos = 0, r = cur_node.node_size_ - 1;
  while(key > cur_node.high_key_) {
    cur_ptr = cur_node.link_ptr_;
    if(cur_ptr.isnull())
      return {nullptr, 0};
    index_fstream_.read(cur_node, cur_ptr);
  }
  while(pos < r) {
    size_t mid = (pos + r) >> 1;
    key_fstream_.read(helper_key, cur_node.key_ptr[mid]);
    if(key <= helper_key)
      r = mid;
    else pos = mid + 1;
  }
  return {cur_ptr, pos};
}


template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::insert(
  const KeyType &key, const ValueType &value) {

  if(root_ptr_.isnull()) {
    // empty tree.

    ValueListNode vlist_node(value);

    IndexNode root_node{true}; // is_leaf_ = true
    root_node.node_size_ = 1;
    root_node.high_key_ = key;
    // root_node.parent_ptr_ is initially nullptr
    // root_node.link_ptr_ is initially nullptr
    root_node.key_ptr[0] = key_fstream_.allocate(key);
    // root_node.child_ptr is initially all nullptr
    root_node.value_list_ptr[0] = vlist_fstream_.allocate(vlist_node);

    root_ptr_ = end_ptr_ = index_fstream_.allocate(root_node);
    // root_ptr doesn't need dynamic update
    return;
  }

  // or we can do some dfs.
  auto [cur_ptr, pos] = upper_bound(key);
  IndexNode cur_node;
  KeyType helper_key;
  bool is_append = false;
  if(cur_ptr.isnull()) {
    // insert at back of tree.
    is_append = true;
    cur_ptr = end_ptr_;
    index_fstream_.read(cur_node, cur_ptr);
    pos = cur_node.node_size_;
  } else {
    index_fstream_.read(cur_node, cur_ptr);
    key_fstream_.read(helper_key, cur_node.key_ptr[pos]);
  }
  if(!is_append && helper_key == key) {
    // this key already exists.
    ValueListPointer cur_vlist_ptr = cur_node.value_list_ptr[pos];
    ValueListNode cur_vlist_node;
    vlist_fstream_.read(cur_vlist_node, cur_vlist_ptr);
    if(value == cur_vlist_node.value_)
      throw BLinkTreeExceptions("Inserting duplicated key-value pair");
    if(value < cur_vlist_node.value_) {
      // insert as the first vlist node
      ValueListNode vlist_node(value);
      vlist_node.next_ptr_ = cur_vlist_ptr;
      cur_node.value_list_ptr[pos] = vlist_fstream_.allocate(vlist_node);
      // cur_node modified.
      index_fstream_.write(cur_node, cur_ptr);
      // no need to check IndexNode split/merge.
      return;
    }
    ValueListPointer prev_vlist_ptr;
    ValueListNode prev_vlist_node;
    while(true){
      prev_vlist_ptr = cur_vlist_ptr;
      prev_vlist_node = cur_vlist_node;
      cur_vlist_ptr = cur_vlist_node.next_ptr_;
      if(cur_vlist_ptr.isnull()) break;
      vlist_fstream_.read(cur_vlist_node, cur_vlist_ptr);
      if(value == cur_vlist_node.value_)
        throw BLinkTreeExceptions("Inserting duplicated key-value pair");
      if(value < cur_vlist_node.value_) {
        // insert between prev_val_ptr and cur_val_ptr
        ValueListNode vlist_node(value);
        vlist_node.next_ptr_ = cur_vlist_ptr;
        ValueListPointer vlist_ptr = vlist_fstream_.allocate(vlist_node);
        prev_vlist_node.next_ptr_ = vlist_ptr;
        // prev_vlist modified.
        vlist_fstream_.write(prev_vlist_node, prev_vlist_ptr);
        // no need to check IndexNode split/merge.
        return;
      }
    }
    // Still not found a bigger one.
    // insert as the last (after prev_vlist_node, the last one)
    ValueListNode vlist_node(value);
    prev_vlist_node.next_ptr_ = vlist_fstream_.allocate(vlist_node);
    // prev_vlist modified.
    vlist_fstream_.write(prev_vlist_node, prev_vlist_ptr);
    // no need to check IndexNode split/merge.
    return;
  }
  // this key doesn't exist yet.
  // insert the new node between pos - 1 and pos.
  for(size_t i = cur_node.node_size_ - 1; i >= pos; --i) {
    cur_node.key_ptr[i + 1] = cur_node.key_ptr[i];
    cur_node.value_list_ptr[i + 1] = cur_node.value_list_ptr[i];
  }
  ValueListNode vlist_node(value);
  cur_node.key_ptr[pos] = key_fstream_.allocate(key);
  cur_node.value_list_ptr[pos] = vlist_fstream_.allocate(vlist_node);
  ++cur_node.node_size_;
  // cur node has changed.
  index_fstream_.write(cur_node, cur_ptr);
  // check if block is to be split.
  if(cur_node.node_size_ == order)
    Split(cur_ptr);
}

template<class KeyType, class ValueType, size_t order, size_t elementCount>
void BLinkTree<KeyType, ValueType, order, elementCount>::erase(const KeyType &key, const ValueType &value) {

}

}