/** blinktree.h
 * Author: StarryPurple
 * Date: Since 2024.12.17
 *
 * Includes some useful data structures;
 *
 * BLink Tree:
 *   A concurrency-friendly data structure widely used in file io.
 *   supports basic functions of a key-value map, similar to std::multimap.
 *
 */

#pragma once
#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#include "filestream.h"

namespace StarryPurple {

// ord for m, the maximum size of a node
// no ValueType is directly used. we only reads and passes fpointer of ValueType.
template<class KeyType, class ValueType, size_t degree = 1 << 10, size_t elementCount = cElementCount>
class BLinkTree {

  struct MapNode;
  struct VlistNode;

public:

  using KeyPtr = typename Fstream<KeyType, size_t, elementCount>::fpointer; // ptr of KeyType node
  using ValuePtr = typename Fstream<ValueType, size_t, elementCount>::fpointer; // ptr of ValurType node
  using VlistPtr = typename Fstream<VlistNode, size_t, elementCount>::fpointer; // ptr of VlistNode
  using MnodePtr = typename Fstream<MapNode, size_t, elementCount>::fpointer; // ptr of MapNode

  BLinkTree() = default;
  ~BLinkTree(); // remember to close files.

  // open the three BLinkTree files.
  // naming rule: with parameter @filename, the names of files would be
  //     filename_map_index.dat, filename_map_key.dat, filename_map_val.dat
  void open(const filenameType &map_filename, const filenameType &key_filename, const filenameType &value_filename);
  // close BLinkTree files
  void close();

  // Which vlist_ptr in which index node is the minimum of those >= @key?
  // if not found (key too big or tree is empty), return {nullptr, 0}
  std::pair<MnodePtr, size_t> upper_bound(const KeyType &key);
  // Which vlist_ptr in which index node is the maximum of those <= @key?
  // if not found (key too big or tree is empty), return {nullptr, 0}
  std::pair<MnodePtr, size_t> lower_bound(const KeyType &key);

  void merge(const MnodePtr &parent_ptr, size_t left_pos); // [1, 2], [5] --> [1, 2, 5]
  void move_from_left(const MnodePtr &parent_ptr, size_t left_pos); // [1, 2, 3, 4], ->[5] --> [1, 2, 3], ->[4, 5]
  void move_from_right(const MnodePtr &parent_ptr, size_t left_pos); // ->[1], [3, 4, 5, 6] --> ->[1, 3], [4, 5, 6]
  void split(const MnodePtr &parent_ptr, size_t pos); // ->[1, 2, 3, 4, 5] --> ->[1, 2], [3, 4, 5]

  void insert(const KeyType &key, const ValueType &value);
  // returns the head vlist node of key.
  // returns "nullptr" if key not found.
  VlistPtr find(const KeyType &key);
  void erase(const KeyType &key, const ValueType &value);

  // todo: iterator
private:
  struct MapNode {

    MapNode() = default;

    bool is_leaf_ = false;
    size_t node_size_ = 0;
    KeyType high_key_{};
    MnodePtr parent_ptr_{}, link_ptr_{};
    KeyPtr key_ptr_[degree + 1]{};
    MnodePtr mnode_ptr_[degree + 1]{}; // nullptr for is_leaf_ = true
    VlistPtr vlist_ptr_[degree + 1]{}; // the begin of the list. nullptr for is_leaf_ = false
    // todo: reduce space cost by using union or variable<MnodePtr, VlistPtr>
  };
  struct VlistNode {
    // a link-list of values.
    VlistNode() = default;
    VlistNode(const ValuePtr &value_ptr, const VlistPtr &next_ptr);
    ~VlistNode() = default;

    ValuePtr value_ptr_{};
    VlistPtr next_ptr_{};
  };

  Fstream<MapNode, MnodePtr, elementCount> map_fstream_;
  Fstream<KeyType, size_t, elementCount> key_fstream_;
  Fstream<ValueType, size_t, elementCount> value_fstream_;
  MnodePtr root_ptr_{};
  bool is_open = false;
};

} // namespace StarryPurple

#endif // DATA_STRUCTURE_H