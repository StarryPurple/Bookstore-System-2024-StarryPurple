#pragma once
#ifndef BLOCK_LIST_H
#define BLOCK_LIST_H

#include "filestream.h"
#include <vector>

namespace StarryPurple {
// Its capacity approximately equals to degree^2.
template<class KeyType, class ValueType, size_t degree>
class BlockList {
  struct HeadNode;
  struct BodyNode;
  struct VListNode;
  using HeadPtr = Fpointer<degree + 1>; // + 1 to distinguish from each other.
  using BodyPtr = Fpointer<degree + 2>; // + 2 to distinguish from each other.
  using VListPtr = Fpointer<degree + 3>; // + 3 to distinguish from each other.
private:
  struct HeadNode {
    KeyType high_key_{};
    HeadPtr nxt_{};
    BodyPtr body_ptr_{};
    int body_len_ = 0;
    HeadNode() = default;
    HeadNode(const KeyType &high_key, const HeadPtr &nxt, const BodyPtr &body_ptr, int body_len);
  };
  struct BodyNode {
    KeyType key_{};
    BodyPtr nxt_{};
    VListPtr vlist_ptr_{};
    BodyNode() = default;
    BodyNode(const KeyType &key, const BodyPtr &nxt, const VListPtr &vlist_ptr);
  };
  struct VListNode {
    ValueType value_{};
    VListPtr nxt_{};
    VListNode() = default;
    VListNode(const ValueType &value, const VListPtr &nxt);
  };
  bool is_open = false;
  HeadPtr begin_head_ptr_{};
  Fstream<HeadNode, HeadPtr, degree + 1> headnode_fstream_; // + 1 to distinguish from each other.
  Fstream<BodyNode, size_t, degree + 2> bodynode_fstream_; // + 2 to distinguish from each other.
  Fstream<VListNode, size_t, degree + 3> vlistnode_fstream_; // + 3 to distinguish from each other.
public:
  BlockList() = default;
  ~BlockList();
  void open(const filenameType &filename_suffix);
  void close();
  void merge(
    const HeadPtr &left_ptr, HeadNode &left_node,
    const HeadPtr &right_ptr, const HeadNode &right_node); // discard right node.
  void average(
    const HeadPtr &left_ptr, HeadNode &left_node,
    const HeadPtr &right_ptr, const HeadNode &right_node);
  void split(const HeadPtr &head_ptr, HeadNode &head_node);
  void insert(const KeyType &key, const ValueType &value);
  void erase(const KeyType &key, const ValueType &value);
  std::vector<ValueType> operator[](const KeyType &key);
};

} // namespace StarryPurple

#endif // BLOCK_LIST_H