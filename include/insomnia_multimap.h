#ifndef INSOMNIA_MULTIMAP_H
#define INSOMNIA_MULTIMAP_H

#include "filestream.h"

#include <thread>
#include <atomic>

namespace Insomnia {

template<class KeyType, class ValueType, size_t capacity, int degree = 32>
class BlocklistMultimap {
  using KVType = std::pair<KeyType, ValueType>;
  using NodePtr = StarryPurple::Fpointer<capacity>;
  struct NodeType {
    NodePtr prev, nxt;
    int node_size = 0;
    KVType kv[degree + 1];
  };

  StarryPurple::Fstream<NodeType, NodePtr, capacity> multimap_fstream;
  NodePtr begin_ptr;
  bool is_open = false;

  void try_average(NodeType &node, NodePtr &node_ptr);
  void merge(NodeType &left_node, NodeType &right_node, NodePtr &left_ptr, NodePtr &right_ptr);
  void average_from_left(NodeType &left_node, NodeType &right_node, NodePtr &left_ptr, NodePtr &right_ptr);
  void average_from_right(NodeType &left_node, NodeType &right_node, NodePtr &left_ptr, NodePtr &right_ptr);
  void split(NodeType &left_node, NodePtr &left_ptr);

public:
  BlocklistMultimap() = default;
  ~BlocklistMultimap();
  void open(const std::string &prefix);
  void close();

  void insert(const KeyType &key, const ValueType &value);
  void erase(const KeyType &key, const ValueType &value);
  std::vector<ValueType> operator[](const KeyType &key);

};
}

#include "insomnia_multimap.tpp"

#endif // INSOMNIA_MULTIMAP_H