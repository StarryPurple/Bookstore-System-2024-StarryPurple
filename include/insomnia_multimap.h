#ifndef INSOMNIA_MULTIMAP_H
#define INSOMNIA_MULTIMAP_H

#include "filestream.h"

#include <thread>
#include <atomic>

namespace Insomnia {

template<class KeyType, class ValueType, int degree, size_t capacity>
class BlinkTree {
  const int largest_size = degree, smallest_size = degree / 2 - 2;
  using KVType = std::pair<KeyType, ValueType>;
  using NodePtr = StarryPurple::Fpointer<capacity>;
  struct NodeType {
    bool is_leaf;
    int node_size;
    NodePtr child[degree + 1], prev, next;
    KVType kv[degree + 1];
  };

  StarryPurple::Fstream<NodeType, NodePtr, capacity> multimap_fstream;
  NodePtr root_ptr;
  bool is_open = false;
  std::vector<std::pair<NodeType, NodePtr>> route;

  void try_average();
  void merge(
    NodePtr &left_ptr, NodeType &left_node, NodePtr &right_ptr, NodeType &right_node,
    NodePtr &parent_ptr, NodeType &parent_node);
  void average_from_left(
    NodePtr &left_ptr, NodeType &left_node, NodePtr &right_ptr, NodeType &right_node,
    NodePtr &parent_ptr, NodeType &parent_node);
  void average_from_right(
    NodePtr &left_ptr, NodeType &left_node, NodePtr &right_ptr, NodeType &right_node,
    NodePtr &parent_ptr, NodeType &parent_node);
  void try_split();

  void maintain_key();

public:
  BlinkTree() = default;
  ~BlinkTree();
  void open(const std::string &prefix);
  void close();

  void insert(const KeyType &key, const ValueType &value);
  void erase(const KeyType &key, const ValueType &value);
  std::vector<ValueType> operator[](const KeyType &key);

};
}

#include "insomnia_multimap.tpp"

#endif // INSOMNIA_MULTIMAP_H