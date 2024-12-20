/** DataStructure.h
 * Author: StarryPurple
 * Date: Since 2024.12.17
 *
 * Includes some useful data structures;
 *
 * BLink Tree:
 *   A concurrency-friendly data structure widely used in file io.
 *   supports basic functions of a key-value map, similar to std::multimap.
 *
 *   (Outdated) Creates 4 files: .mapdat, .vallistdat, .keydat, .valdat
 *   .mapdat for IndexNode, .vallistdat for ValueListType, .keydat for KeyType, .valdat for ValueType
 *
 *
 *
 */

#pragma once
#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#include "FileStream.h"

namespace StarryPurple {

// ord for m, the maximum size of a node
template<class KeyType, class ValueType, size_t order = 1 << 10, size_t elementCount = cElementCount>
class BLinkTree {

  struct IndexNode;
  struct ValueListNode;

public:

  using KeyPointer = typename Fstream<KeyType, size_t, elementCount>::fpointer;
  // using ValuePointer = typename Fstream<ValueType, size_t, elementCount>::fpointer;
  using ValueListPointer = typename Fstream<ValueListNode, size_t, elementCount>::fpointer;
  using IndexPointer = typename Fstream<IndexNode, size_t, elementCount>::fpointer;

  BLinkTree() = default;
  ~BLinkTree(); // remember to close files.

  // open the three BLinkTree files.
  // naming rule: with parameter @filename, the names of files would be
  //     filename_map_index.dat, filename_map_key.dat, filename_map_val.dat
  void open(const filenameType &filename);
  // close BLinkTree files
  void close();

  void MergeLeft(const IndexPointer &index_ptr);
  void MergeRight(const IndexPointer &index_ptr);
  void Split(const IndexPointer &index_ptr);

  // Which vlist_ptr in which index node is our target?
  // if not found (key too big or tree is empty), return {nullptr, 0}
  std::pair<IndexPointer, size_t> upper_bound(const KeyType &key);

  void insert(const KeyType &key, const ValueType &value);
  // returns the head vlist node of key.
  // returns "nullptr" if key not found.
  ValueListPointer find(const KeyType &key);
  void erase(const KeyType &key, const ValueType &value);

private:
  struct IndexNode {

    IndexNode() = delete;
    IndexNode(bool is_leaf);

    const bool is_leaf_ = false;
    size_t node_size_ = 0;
    KeyType high_key_{};
    IndexPointer parent_ptr_{}, link_ptr_{};
    KeyPointer key_ptr[order]{};
    IndexPointer child_index_ptr_[order]{}; // nullptr for is_leaf_ = true
    ValueListPointer value_list_ptr[order]{}; // the begin of the list. nullptr for is_leaf_ = false
    // todo: the iterator ptr (maybe link_ptr_ is sufficient enough?)
  };
  struct ValueListNode {
    // a link-list of values.
    ValueListNode(const ValueType &value);
    ~ValueListNode() = default;

    ValueType value_{};
    ValueListPointer next_ptr_{};
  };
  struct InfoType {
    IndexPointer root_ptr_, end_ptr;
  };
  Fstream<IndexNode, InfoType, elementCount> index_fstream_;
  Fstream<KeyType, size_t, elementCount> key_fstream_;
  Fstream<ValueListNode, size_t, elementCount> vlist_fstream_;
  IndexPointer root_ptr_{}, end_ptr_{};
  filenameType filename_;
  bool is_open = false;
};

} // namespace StarryPurple

#endif // DATA_STRUCTURE_H