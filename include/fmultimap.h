/** fmultimap.h
 * Author: StarryPurple
 * Date: Since 2024.12.17
 *
 * Includes some useful data structures;
 *
 * fmultimap / B Plus Tree:
 *   A (concurrency-friendly) data structure widely used in file io.
 *   supports basic functions of a key-value map similar to std::multimap, but with file storage.
 */

#pragma once
#ifndef F_MULTIMAP_H
#define F_MULTIMAP_H

#include "filestream.h"
#include "lrucache.h"

#include <vector>
#include <utility>

namespace StarryPurple {

// degree for the maximum size of a node
// node_size should be in [degree / 2 - 1, degree - 1]
// no ValueType is directly used. we only reads and passes fpointer of ValueType.
template<class KeyType, class ValueType, size_t degree = 1 << 7, size_t capacity = cCapacity>
class Fmultimap {
  struct InnerNode;
  using InnerPtr = Fpointer<2 * capacity / degree + 10>;
  using InnerFstream = Fstream<InnerNode, InnerPtr, 2 * capacity / degree + 10>;
  using KVPairType = std::pair<KeyType, ValueType>;
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
  /*
  struct VlistNode {
    // todo: add this_ptr
    ValueType value;
    VlistPtr nxt;
  };*/
  InnerFstream inner_fstream;
  bool is_open = false;
  InnerPtr root_ptr; // parent_ptr of root_node is "nullptr"
public:
  Fmultimap() = default;
  ~Fmultimap();
  void open(const filenameType &name_suffix);
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

} // namespace StarryPurple

#endif // F_MULTIMAP_H