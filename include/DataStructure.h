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
 *   Creates 3 files: .mapdat, .keydat, .valdat
 *   .mapdat for IndexNode, .keydat for KeyType, .valdat for ValueType
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

  class IndexNode;

public:

  using KeyPointer = typename Fstream<KeyType, size_t, elementCount>::fpointer;
  using ValuePointer = typename Fstream<ValueType, size_t, elementCount>::fpointer;
  using IndexPointer = typename Fstream<IndexNode, size_t, elementCount>::fpointer;

  BLinkTree() = default;
  ~BLinkTree(); // remember to close files.

  // open the three BLinkTree files.
  // naming rule: with parameter @filename, the names of files would be
  //     filename_map_index.dat, filename_map_key.dat, filename_map_val.dat
  void open(const filenameType &filename);
  // close BLinkTree files
  void close();

  void insert(const KeyType &key, const ValueType &value);
  // returns "nullptr" if value not found.
  ValuePointer find(const ValueType &value);
  void erase(const KeyType &key, const ValueType &value);

private:
  struct IndexNode {

    IndexNode() = delete;
    IndexNode(bool is_leaf);
    ~IndexNode() = default;

    static void InsertNonLeaf(const KeyType &key, const KeyPointer &key_ptr);
    static void InsertLeaf(
      const KeyType &key, const KeyPointer &key_ptr,
      const ValueType &value, const ValuePointer &value_ptr);
    static void MergeRight(const IndexPointer &index_ptr);
    static void Split(const IndexPointer &index_ptr);

    const bool is_leaf_ = false;
    size_t node_size_ = 0;
    KeyType high_key_{};
    IndexPointer parent_ptr_{}, link_ptr_{};
    KeyPointer key_ptr[order + 1]{};
    IndexPointer child_index_ptr_[order + 1]{}; // nullptr for is_leaf_ = true
    ValuePointer value_ptr[order + 1]{}; // nullptr for is_leaf_ = false
    // todo: the iterator ptr (maybe link_ptr_ is sufficient enough?)
  };
  Fstream<IndexNode, IndexPointer, elementCount> index_fstream_;
  Fstream<KeyType, size_t, elementCount> key_fstream_;
  Fstream<ValueType, size_t, elementCount> value_fstream_;
  IndexPointer root_ptr_;
  filenameType filename_;
  bool is_open = false;
};

} // namespace StarryPurple

#endif // DATA_STRUCTURE_H