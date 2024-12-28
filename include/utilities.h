#pragma once
#ifndef UTILITIES_H
#define UTILITIES_H

#include "filestream.h"
#include "lrucache.h"
#include "validator.h"

#include <vector>
#include <utility>

namespace StarryPurple {

// degree for the maximum size of a node
// node_size should be in [degree / 2 - 1, degree - 1]
// no ValueType is directly used. we only reads and passes fpointer of ValueType.
template<class KeyType, class ValueType, size_t degree, size_t capacity>
class Fmultimap {
  struct InnerNode;
  struct VlistNode;
  using InnerPtr = Fpointer<2 * capacity / degree + 10>;
  using InnerFstream = Fstream<InnerNode, InnerPtr, 2 * capacity / degree + 10>;
  using KVPairType = std::pair<KeyType, ValueType>;
  using VlistPtr = Fpointer<capacity>;
  using VlistFstream = Fstream<VlistNode, size_t, capacity>;
private:
  struct InnerNode {
    // todo: add this_ptr
    bool is_leaf = false;
    InnerPtr link_ptr;
    KVPairType high_kv;
    InnerPtr parent_ptr;
    size_t node_size = 0;
    KVPairType kv_pairs[degree + 1];
    InnerPtr inner_ptrs[degree + 1];
  };
  struct VlistNode {
    // todo: add this_ptr
    ValueType value;
    VlistPtr nxt;
  };
  InnerFstream inner_fstream;
  VlistFstream vlist_fstream;
  bool is_open = false;
  InnerPtr root_ptr; // parent_ptr of root_node is "nullptr"
  LRUCache<KeyType, VlistPtr, 20> vlist_begin_cache;
public:
  Fmultimap() = default;
  ~Fmultimap();
  void open(const filenameType &prefix);
  void close();

  void insert(const KeyType &key, const ValueType &value);
  void erase(const KeyType &key, const ValueType &value);
  std::vector<ValueType> operator[](const KeyType &key);

  // after splitting, split_node will become its parent node,
  // split_ptr will become the pointer of the original split_node.
  // Attention: the parent_ptr of split_node.inner_nodes won't be updated here.
  void split(
    size_t split_pos,
     InnerPtr &split_ptr, InnerNode &split_node);
  /*
  void merge(
    const InnerPtr &parent_ptr,
    size_t left_pos,
    const InnerNode &left_node, const InnerNode &right_node);
  void average_left(
    const InnerPtr &parent_ptr,
    size_t left_pos,
    const InnerNode &left_node, const InnerNode &right_node);
  void average_right(
    const InnerPtr &parent_ptr,
    size_t left_pos,
    const InnerNode &left_node, const InnerNode &right_node);
    */
  // after maintenance, maintain_node will become its parent node.
  // when there's only split operation.
  void maintain_size(
    InnerPtr &maintain_ptr, InnerNode &maintain_node);

};

template<class Type, size_t capacity>
class Fstack {
  using ListNodePtr = Fpointer<capacity>;
  struct ListNodeType {
    Type val;
    ListNodePtr pre;
  };
  struct InfoType {
    ListNodeType back_node;
    ListNodePtr back_ptr;
    size_t current_size = 0;
  };
  using StackFstream = Fstream<ListNodeType, InfoType, capacity>;
private:
  StackFstream stack_fstream;
  bool is_open = false;
  InfoType info;
public:
  Fstack() = default;
  ~Fstack();
  void open(const filenameType &prefix);
  void close();
  void push(const Type &value);
  void pop();
  Type &top();
  bool empty() const;
  size_t size() const;
};

template<int capacity>
class ConstStr {
private:
  char storage[capacity + 1];
  size_t len;
public:
  ConstStr();
  ~ConstStr() = default;
  ConstStr(const std::string &str);
  ConstStr(const ConstStr &other);
  bool operator==(const ConstStr &other) const;
  bool operator!=(const ConstStr &other) const;
  bool empty() const;
};

} // namespace StarryPurple


#include "utilities.tpp"

#endif // UTILITIES_H