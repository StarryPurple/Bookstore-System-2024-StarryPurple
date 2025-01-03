#ifndef INSOMNIA_MULTIMAP_TPP
#define INSOMNIA_MULTIMAP_TPP

#include "insomnia_multimap.h"


template<class KeyType, class ValueType, int degree, size_t capacity>
Insomnia::BlinkTree<KeyType, ValueType, degree, capacity>::~BlinkTree() {
  if(is_open) close();
}

template<class KeyType, class ValueType, int degree, size_t capacity>
void Insomnia::BlinkTree<KeyType, ValueType, degree, capacity>::open(const std::string &prefix) {
  if(is_open) close();
  bool is_exist = multimap_fstream.open(prefix + "_multimap.bsdat");
  if(is_exist) multimap_fstream.read_info(root_ptr);
  else {
    NodeType node;
    node.is_leaf = true; node.node_size = 0;
    node.next.setnull(); node.prev.setnull();
    root_ptr = multimap_fstream.allocate(node);
  }
  is_open = true;
}

template<class KeyType, class ValueType, int degree, size_t capacity>
void Insomnia::BlinkTree<KeyType, ValueType, degree, capacity>::close() {
  if(!is_open) return;
  multimap_fstream.write_info(root_ptr);
  multimap_fstream.close();
  is_open = false;
}

template<class KeyType, class ValueType, int degree, size_t capacity>
void Insomnia::BlinkTree<KeyType, ValueType, degree, capacity>::insert(const KeyType &key, const ValueType &value) {
  KVType kv_pair = {key, value};
  route.clear();
  NodePtr cur_ptr = root_ptr;
  NodeType cur_node; multimap_fstream.read(cur_node, cur_ptr);
  if(cur_node.node_size == 0) {
    // empty
    cur_node.node_size = 1;
    cur_node.kv[0] = kv_pair;
    multimap_fstream.write(cur_node, cur_ptr);
    return;
  }
  while(true) {
    if(cur_node.is_leaf) {
      if(kv_pair > cur_node.kv[cur_node.node_size - 1]) {
        cur_node.kv[cur_node.node_size] = kv_pair;
        ++cur_node.node_size;
        multimap_fstream.write(cur_node, cur_ptr);
        route.push_back({cur_node, cur_ptr});
        break;
      }
      int l = 0, r = cur_node.node_size - 1;
      while(l < r) {
        int mid = (l + r) >> 1;
        if(kv_pair <= cur_node.kv[mid]) r = mid;
        else l = mid + 1;
      }
      if(kv_pair == cur_node.kv[l]) return;
      for(int i = cur_node.node_size; i > l; --i)
        cur_node.kv[i] = cur_node.kv[i - 1];
      cur_node.kv[l] = kv_pair;
      ++cur_node.node_size;
      multimap_fstream.write(cur_node, cur_ptr);
      route.push_back({cur_node, cur_ptr});
      break;
    }
    if(kv_pair > cur_node.kv[cur_node.node_size - 1]) {
      cur_node.kv[cur_node.node_size - 1] = kv_pair;
      multimap_fstream.write(cur_node, cur_ptr);
      route.push_back({cur_node, cur_ptr});
      cur_ptr = cur_node.child[cur_node.node_size - 1];
      multimap_fstream.read(cur_node, cur_ptr);
      continue;
    }
    int l = 0, r = cur_node.node_size - 1;
    while(l < r) {
      int mid = (l + r) >> 1;
      if(kv_pair <= cur_node.kv[mid]) r = mid;
      else l = mid + 1;
    }
    route.push_back({cur_node, cur_ptr});
    cur_ptr = cur_node.child[l];
    multimap_fstream.read(cur_node, cur_ptr);
  }
  try_split();
}

template<class KeyType, class ValueType, int degree, size_t capacity>
void Insomnia::BlinkTree<KeyType, ValueType, degree, capacity>::erase(const KeyType &key, const ValueType &value) {
  KVType kv_pair = {key, value};
  route.clear();
  NodePtr cur_ptr = root_ptr;
  NodeType cur_node; multimap_fstream.read(cur_node, cur_ptr);
  if(cur_node.node_size == 0) return; // empty
  if(kv_pair > cur_node.kv[cur_node.node_size - 1]) return; // too large
  while(true) {
    if(cur_node.is_leaf) {
      int l = 0, r = cur_node.node_size - 1;
      while(l < r) {
        int mid = (l + r) >> 1;
        if(kv_pair <= cur_node.kv[mid]) r = mid;
        else l = mid + 1;
      }
      if(kv_pair != cur_node.kv[l]) return;
      --cur_node.node_size;
      for(int i = l; i < cur_node.node_size; ++i)
        cur_node.kv[i] = cur_node.kv[i + 1];
      multimap_fstream.write(cur_node, cur_ptr);
      route.push_back({cur_node, cur_ptr});
      break;
    }
    int l = 0, r = cur_node.node_size - 1;
    while(l < r) {
      int mid = (l + r) >> 1;
      if(kv_pair <= cur_node.kv[mid]) r = mid;
      else l = mid + 1;
    }
    route.push_back({cur_node, cur_ptr});
    cur_ptr = cur_node.child[l];
    multimap_fstream.read(cur_node, cur_ptr);
  }
  try_average();
}

template<class KeyType, class ValueType, int degree, size_t capacity>
std::vector<ValueType> Insomnia::BlinkTree<KeyType, ValueType, degree, capacity>::operator[](const KeyType &key) {
  std::vector<ValueType> list;
  NodePtr cur_ptr = root_ptr;
  NodeType cur_node; multimap_fstream.read(cur_node, cur_ptr);
  if(cur_node.node_size == 0) return list; // empty
  if(key > cur_node.kv[cur_node.node_size - 1].first) return list; // too large
  while(true) {
    int l = 0, r = cur_node.node_size - 1;
    while(l < r) {
      int mid = (l + r) >> 1;
      if(key <= cur_node.kv[mid].first) r = mid;
      else l = mid + 1;
    }
    if(!cur_node.is_leaf) {
      cur_ptr = cur_node.child[l];
      multimap_fstream.read(cur_node, cur_ptr);
      continue;
    }
    int start = l;
    while(true) {
      for(int i = start; i < cur_node.node_size; ++i) {
        if(key == cur_node.kv[i].first) list.push_back(cur_node.kv[i].second);
        else return list;
      }
      cur_ptr = cur_node.next;
      if(cur_ptr.isnull()) return list;
      multimap_fstream.read(cur_node, cur_ptr);
      start = 0;
    }
  }
}

template<class KeyType, class ValueType, int degree, size_t capacity>
void Insomnia::BlinkTree<KeyType, ValueType, degree, capacity>::try_split() {
  if(route.back().first.node_size <= largest_size) return;
  NodeType cur_node = route.back().first, parent_node;
  NodePtr cur_ptr = route.back().second, parent_ptr;
  route.pop_back();
  if(!route.empty()) {
    parent_node = route.back().first;
    parent_ptr = route.back().second;
    route.pop_back();
  } else {
    // new root
    parent_node.node_size = 1;
    parent_node.is_leaf = false;
    parent_node.kv[0] = cur_node.kv[cur_node.node_size - 1];
    parent_node.child[0] = cur_ptr;
    root_ptr = parent_ptr = multimap_fstream.allocate(parent_node);
  }

  NodeType right_node;
  int left_size = cur_node.node_size / 2, right_size = cur_node.node_size - left_size;
  for(int i = 0; i < right_size; ++i) {
    right_node.child[i] = cur_node.child[left_size + i];
    right_node.kv[i] = cur_node.kv[left_size + i];
  }
  cur_node.node_size = left_size;
  right_node.node_size = right_size;
  right_node.is_leaf = cur_node.is_leaf;
  right_node.next = cur_node.next;
  right_node.prev = cur_ptr;
  NodePtr right_ptr = multimap_fstream.allocate(right_node);
  if(!cur_node.next.isnull()) {
    NodeType rr_node;
    multimap_fstream.read(rr_node, cur_node.next);
    rr_node.prev = right_ptr;
    multimap_fstream.write(rr_node, cur_node.next);
  }
  cur_node.next = right_ptr;
  multimap_fstream.write(cur_node, cur_ptr);

  KVType kv_pair = right_node.kv[right_node.node_size - 1];
  int l = 0, r = parent_node.node_size;
  while(l < r) {
    int mid = (l + r) >> 1;
    if(kv_pair > parent_node.kv[mid]) l = mid + 1;
    else r = mid;
  }
  // assert(kv_pair == parent_node.kv[l]);
  for(int i = parent_node.node_size; i > l; --i) {
    parent_node.kv[i] = parent_node.kv[i - 1];
    parent_node.child[i] = parent_node.child[i - 1];
  }
  ++parent_node.node_size;
  parent_node.kv[l] = cur_node.kv[cur_node.node_size - 1];
  parent_node.child[l + 1] = right_ptr;
  parent_node.child[l] = cur_ptr;
  multimap_fstream.write(parent_node, parent_ptr);

  route.push_back({parent_node, parent_ptr});
  try_split();
}

template<class KeyType, class ValueType, int degree, size_t capacity>
void Insomnia::BlinkTree<KeyType, ValueType, degree, capacity>::try_average() {
  if(route.back().first.node_size >= smallest_size) return;
  NodeType cur_node = route.back().first;
  NodePtr cur_ptr = route.back().second;
  route.pop_back();
  if(route.empty()) {
    // cur_node = root node.
    // the root node has too little nodes.
    if(cur_node.is_leaf)
      // The tree has only one node (root node).
      // We have to accept this.
      return;
    // else we should ask:
    if(cur_node.node_size > 1)
      return; // It can be accepted.
    // delete the root node.
    root_ptr = cur_node.child[0];
    multimap_fstream.free(cur_ptr);
    return;
  }
  NodeType parent_node = route.back().first;
  NodePtr parent_ptr = route.back().second;
  route.pop_back();

  bool has_left = !cur_node.prev.isnull(), has_right = !cur_node.next.isnull();
  NodeType left_node, right_node;
  NodePtr left_ptr, right_ptr;
  while(true) {
    if(has_left) {
      left_ptr = cur_node.prev;
      multimap_fstream.read(left_node, left_ptr);
      if(left_node.node_size + cur_node.node_size < largest_size) {
        merge(left_ptr, left_node, cur_ptr, cur_node, parent_ptr, parent_node);
        break;
      }
    }
    if(has_right) {
      right_ptr = cur_node.next;
      multimap_fstream.read(right_node, right_ptr);
      if(right_node.node_size + cur_node.node_size < largest_size) {
        merge(cur_ptr, cur_node, right_ptr, right_node, parent_ptr, parent_node);
        break;
      }
    }
    if(has_left && has_right) {
      if(left_node.node_size > right_node.node_size)
        average_from_left(left_ptr, left_node, cur_ptr, cur_node, parent_ptr, parent_node);
      else
        average_from_right(cur_ptr, cur_node, right_ptr, right_node, parent_ptr, parent_node);
      break;
    }
    if(has_left)
      average_from_left(left_ptr, left_node, cur_ptr, cur_node, parent_ptr, parent_node);
    else
      average_from_right(cur_ptr, cur_node, right_ptr, right_node, parent_ptr, parent_node);
    break;
  }

  route.push_back({parent_node, parent_ptr});
  try_average();
}

template<class KeyType, class ValueType, int degree, size_t capacity>
void Insomnia::BlinkTree<KeyType, ValueType, degree, capacity>::merge(
  NodePtr &left_ptr, NodeType &left_node, NodePtr &right_ptr, NodeType &right_node,
  NodePtr &parent_ptr, NodeType &parent_node) {
  int left_size = left_node.node_size, rifht_size = right_node.node_size;
  KVType kv_pair = left_node.kv[left_node.node_size - 1];
  for(int i = 0; i < rifht_size; ++i) {
    left_node.kv[left_size + i] = right_node.kv[i];
    left_node.child[left_size + i] = right_node.child[i];
  }
  left_node.node_size = left_size + rifht_size;
  left_node.next = right_node.next;
  if(!right_node.next.isnull()) {
    NodeType rr_node;
    multimap_fstream.read(rr_node, right_node.next);
    rr_node.prev = left_ptr;
    multimap_fstream.write(rr_node, right_node.next);
  }
  multimap_fstream.write(left_node, left_ptr);
  multimap_fstream.free(right_ptr);

  int l = 0, r = parent_node.node_size - 1;
  while(l < r) {
    int mid = (l + r) >> 1;
    if(kv_pair > parent_node.kv[mid]) l = mid + 1;
    else r = mid;
  }
  --parent_node.node_size;
  for(int i = l; i < parent_node.node_size; ++i) {
    parent_node.kv[i] = parent_node.kv[i + 1];
    parent_node.child[i] = parent_node.child[i + 1];
  }
  parent_node.child[l] = left_ptr;
  multimap_fstream.write(parent_node, parent_ptr);
}

template<class KeyType, class ValueType, int degree, size_t capacity>
void Insomnia::BlinkTree<KeyType, ValueType, degree, capacity>::average_from_left(
  NodePtr &left_ptr, NodeType &left_node, NodePtr &right_ptr, NodeType &right_node,
  NodePtr &parent_ptr, NodeType &parent_node) {
  int total_size = left_node.node_size + right_node.node_size;
  KVType kv_pair = left_node.kv[left_node.node_size - 1];
  int left_size = total_size / 2, right_size = total_size - left_size;
  int diff = left_node.node_size - left_size;
  // assert...
  for(int i = right_node.node_size; i >= 0; --i) {
    right_node.kv[diff + i] = right_node.kv[i];
    right_node.child[diff + i] = right_node.child[i];
  }
  for(int i = 0; i < diff; ++i) {
    right_node.kv[i] = left_node.kv[left_size + i];
    right_node.child[i] = left_node.child[left_size + i];
  }
  left_node.node_size = left_size;
  right_node.node_size = right_size;
  multimap_fstream.write(left_node, left_ptr);
  multimap_fstream.write(right_node, right_ptr);

  int l = 0, r = parent_node.node_size - 1;
  while(l < r) {
    int mid = (l + r) >> 1;
    if(kv_pair > parent_node.kv[mid]) l = mid + 1;
    else r = mid;
  }
  parent_node.kv[l] = left_node.kv[left_node.node_size - 1];
  multimap_fstream.write(parent_node, parent_ptr);
}

template<class KeyType, class ValueType, int degree, size_t capacity>
void Insomnia::BlinkTree<KeyType, ValueType, degree, capacity>::average_from_right(
  NodePtr &left_ptr, NodeType &left_node, NodePtr &right_ptr, NodeType &right_node,
  NodePtr &parent_ptr, NodeType &parent_node) {
  int total_size = left_node.node_size + right_node.node_size;
  KVType kv_pair = left_node.kv[left_node.node_size - 1];
  int left_size = total_size / 2, right_size = total_size - left_size;
  int diff = left_size - left_node.node_size;
  for(int i = 0; i < diff; ++i) {
    left_node.kv[left_node.node_size + i] = right_node.kv[i];
    left_node.child[left_node.node_size + i] = right_node.child[i];
  }
  for(int i = 0; i < right_size; ++i) {
    right_node.kv[i] = right_node.kv[diff + i];
    right_node.child[i] = right_node.child[diff + i];
  }
  left_node.node_size = left_size;
  right_node.node_size = right_size;
  multimap_fstream.write(left_node, left_ptr);
  multimap_fstream.write(right_node, right_ptr);

  int l = 0, r = parent_node.node_size - 1;
  while(l < r) {
    int mid = (l + r) >> 1;
    if(kv_pair > parent_node.kv[mid]) l = mid + 1;
    else r = mid;
  }
  parent_node.kv[l] = left_node.kv[left_node.node_size - 1];
  multimap_fstream.write(parent_node, parent_ptr);
}



#endif // INSOMNIA_MULTIMAP_TPP