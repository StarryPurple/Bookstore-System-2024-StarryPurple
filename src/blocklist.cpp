

#include "blocklist.h"

namespace StarryPurple {

template<class KeyType, class ValueType, size_t degree>
BlockList<KeyType, ValueType, degree>::HeadNode::HeadNode(
  const KeyType &high_key, const HeadPtr &nxt, const BodyPtr &body_ptr, int body_len)
  : high_key_(high_key), nxt_(nxt), body_ptr_(body_ptr), body_len_(body_len) {}

template<class KeyType, class ValueType, size_t degree>
BlockList<KeyType, ValueType, degree>::BodyNode::BodyNode(
  const KeyType &key, const BodyPtr &nxt, const VListPtr &vlist_ptr)
  : key_(key), nxt_(nxt), vlist_ptr_(vlist_ptr) {}

template<class KeyType, class ValueType, size_t degree>
BlockList<KeyType, ValueType, degree>::VListNode::VListNode(const ValueType &value, const VListPtr &nxt)
  : value_(value), nxt_(nxt) {}


template<class KeyType, class ValueType, size_t degree>
BlockList<KeyType, ValueType, degree>::~BlockList() {
  if(is_open)
    close();
}

template<class KeyType, class ValueType, size_t degree>
void BlockList<KeyType, ValueType, degree>::open(const filenameType &filename_suffix) {
  is_open = true;
  headnode_fstream_.open(filename_suffix + "_map_head.bsdat");
  bodynode_fstream_.open(filename_suffix + "_map_list.bsdat");
  vlistnode_fstream_.open(filename_suffix + "_map_vlist.bsdat");
  headnode_fstream_.read_info(begin_head_ptr_);
}

template<class KeyType, class ValueType, size_t degree>
void BlockList<KeyType, ValueType, degree>::close() {
  headnode_fstream_.write_info(begin_head_ptr_);
  headnode_fstream_.close();
  bodynode_fstream_.close();
  vlistnode_fstream_.close();
  is_open = false;
}

template<class KeyType, class ValueType, size_t degree>
void BlockList<KeyType, ValueType, degree>::insert(const KeyType &key, const ValueType &value) {
  if(begin_head_ptr_.isnull()) {
    // empty
    VListPtr new_vlist_ptr = vlistnode_fstream_.allocate({value, VListPtr()});
    VListPtr new_vlist_begin = vlistnode_fstream_.allocate({ValueType(), new_vlist_ptr});
    BodyPtr new_body_ptr = bodynode_fstream_.allocate({key, BodyPtr(), new_vlist_begin});
    BodyPtr new_body_begin = bodynode_fstream_.allocate({KeyType(), new_body_ptr, VListPtr()});
    HeadPtr new_head_ptr = headnode_fstream_.allocate({key, HeadPtr(), new_body_begin, 1});
    begin_head_ptr_ = headnode_fstream_.allocate({KeyType(), new_head_ptr, BodyPtr(), 0});
    return;
  }
  HeadPtr cur_head_ptr = begin_head_ptr_, nxt_head_ptr;
  HeadNode cur_head_node, nxt_head_node;
  headnode_fstream_.read(cur_head_node, cur_head_ptr); // must exist as the empty begin node
  nxt_head_ptr = cur_head_node.nxt_;
  while(!nxt_head_ptr.isnull()) {
    headnode_fstream_.read(nxt_head_node, nxt_head_ptr);
    if(key > nxt_head_node.high_key_) {
      cur_head_node = nxt_head_node;
      cur_head_ptr = nxt_head_ptr;
      nxt_head_ptr = cur_head_node.nxt_;
      continue;
    }
    // insert in nxt_head
    BodyPtr cur_body_ptr = nxt_head_node.body_ptr_, nxt_body_ptr;
    BodyNode cur_body_node, nxt_body_node;
    bodynode_fstream_.read(cur_body_node, cur_body_ptr); // must exist as an empty begin node
    nxt_body_ptr = cur_body_node.nxt_;
    while(!nxt_body_ptr.isnull()) {
      bodynode_fstream_.read(nxt_body_node, nxt_body_ptr);
      if(key > nxt_body_node.key_) {
        cur_body_ptr = nxt_body_ptr;
        cur_body_node = nxt_body_node;
        nxt_body_ptr = cur_body_node.nxt_;
        continue;
      }
      if(key < nxt_body_node.key_) {
        // insert a new key between cur_body and nxt_body
        VListPtr new_vlist_ptr = vlistnode_fstream_.allocate({value, VListPtr()});
        VListPtr new_vlist_begin = vlistnode_fstream_.allocate({ValueType(), new_vlist_ptr});
        cur_body_node.nxt_ = bodynode_fstream_.allocate({key, nxt_body_ptr, new_vlist_begin});
        bodynode_fstream_.write(cur_body_node, cur_body_ptr);
        break;
      }
      if(key == nxt_body_node.key_) {
        // insert a new value in the vlist of nxt_body
        VListPtr cur_vlist_ptr = nxt_body_node.vlist_ptr_, nxt_vlist_ptr;
        VListNode cur_vlist_node, nxt_vlist_node;
        vlistnode_fstream_.read(cur_vlist_node, cur_vlist_ptr); // must exist as an empty begin node
        nxt_vlist_ptr = cur_vlist_node.nxt_;
        bool has_inserted = false;
        while(!nxt_vlist_ptr.isnull()) {
          vlistnode_fstream_.read(nxt_vlist_node, nxt_vlist_ptr);
          if(value > nxt_vlist_node.value_) {
            cur_vlist_ptr = nxt_vlist_ptr;
            cur_vlist_node = nxt_vlist_node;
            nxt_vlist_ptr = cur_vlist_node.nxt_;
            continue;
          }
          if(value == nxt_vlist_node.value_) {
            // duplicated value. take it as insertion completed.
            has_inserted = true;
            break;
          }
          if(value < nxt_vlist_node.value_) {
            // insert between cur_vlist and nxt_vlist
            cur_vlist_node.nxt_ = vlistnode_fstream_.allocate({value, nxt_vlist_ptr});
            vlistnode_fstream_.write(cur_vlist_node, cur_vlist_ptr);
            has_inserted = true;
            break;
          }
        }
        if(!has_inserted) {
          // cur_vlist as the end of the vlist, while nxt_vlist_ptr == nullptr.
          // insert after cur_vlist. Just like what may happen when inserting midway.
          cur_vlist_node.nxt_ = vlistnode_fstream_.allocate({value, nxt_vlist_ptr});
          vlistnode_fstream_.write(cur_vlist_node, cur_vlist_ptr);
        }
        break;
      }
    }
    // insertion must have been completed, for key <= high_key_.
    ++nxt_head_node.body_len_;
    headnode_fstream_.write(nxt_head_node, nxt_head_ptr);
    // attention: head_node split may happen.
    if(nxt_head_node.body_len_ == degree)
      split(nxt_head_ptr, nxt_head_node);
    return;
  }
  // key is larger than any key existing.
  // cur_head is back of head nodes.
  // try to insert it at the back of head_node list.
  // cur_head modified.
  BodyPtr cur_body_ptr = cur_head_node.body_ptr_, nxt_body_ptr;
  BodyNode cur_body_node, nxt_body_node;
  bodynode_fstream_.read(cur_body_node, cur_body_ptr);
  nxt_body_ptr = cur_body_node.nxt_;
  while(!nxt_body_ptr.isnull()) {
    bodynode_fstream_.read(nxt_body_node, nxt_body_ptr);
    cur_body_ptr = nxt_body_ptr;
    cur_body_node = nxt_body_node;
    nxt_body_ptr = cur_body_node.nxt_;
  }
  // append at end of the body list.
  VListPtr new_vlist_ptr = vlistnode_fstream_.allocate({value, VListPtr()});
  VListPtr new_vlist_begin = vlistnode_fstream_.allocate({ValueType(), new_vlist_ptr});
  cur_body_node.nxt_ = bodynode_fstream_.allocate({key, BodyPtr(), new_vlist_begin});
  bodynode_fstream_.write(cur_body_node, cur_body_ptr);

  ++cur_head_node.body_len_;
  cur_head_node.high_key_ = key;
  headnode_fstream_.write(cur_head_node, cur_head_ptr);
  if(cur_head_node.body_len_ == degree)
    split(cur_head_ptr, cur_head_node);
}

template<class KeyType, class ValueType, size_t degree>
void BlockList<KeyType, ValueType, degree>::erase(const KeyType &key, const ValueType &value) {
  if(begin_head_ptr_.isnull())
    return; // empty
  HeadPtr cur_head_ptr = begin_head_ptr_, nxt_head_ptr;
  HeadNode cur_head_node, nxt_head_node;
  headnode_fstream_.read(cur_head_node, cur_head_ptr); // must exist as the empty begin node
  nxt_head_ptr = cur_head_node.nxt_;
  while(!nxt_head_ptr.isnull()) {
    headnode_fstream_.read(nxt_head_node, nxt_head_ptr);
    if(key > nxt_head_node.high_key_) {
      cur_head_node = nxt_head_node;
      cur_head_ptr = nxt_head_ptr;
      nxt_head_ptr = cur_head_node.nxt_;
      continue;
    }
    // search in nxt_head
    BodyPtr cur_body_ptr = nxt_head_node.body_ptr_, nxt_body_ptr;
    BodyNode cur_body_node, nxt_body_node;
    bodynode_fstream_.read(cur_body_node, cur_body_ptr); // must exist as an empty begin node
    nxt_body_ptr = cur_body_node.nxt_;
    while(!nxt_body_ptr.isnull()) {
      bodynode_fstream_.read(nxt_body_node, nxt_body_ptr);
      if(key > nxt_body_node.key_) {
        cur_body_ptr = nxt_body_ptr;
        cur_body_node = nxt_body_node;
        nxt_body_ptr = cur_body_node.nxt_;
        continue;
      }
      if(key < nxt_body_node.key_) {
        // key not found.
        return;
      }
      if(key == nxt_body_node.key_) {
        // key found in nxt_body_node.
        VListPtr cur_vlist_ptr = nxt_body_node.vlist_ptr_, nxt_vlist_ptr;
        VListNode cur_vlist_node, nxt_vlist_node;
        vlistnode_fstream_.read(cur_vlist_node, cur_vlist_ptr); // must exist as an empty begin node
        nxt_vlist_ptr = cur_vlist_node.nxt_;
        while(!nxt_vlist_ptr.isnull()) {
          vlistnode_fstream_.read(nxt_vlist_node, nxt_vlist_ptr);
          if(value > nxt_vlist_node.value_) {
            cur_vlist_ptr = nxt_vlist_ptr;
            cur_vlist_node = nxt_vlist_node;
            nxt_vlist_ptr = cur_vlist_node.nxt_;
            continue;
          }
          if(value == nxt_vlist_node.value_) {
            // nxt_vlist contains the value to erase.
            cur_vlist_node.nxt_ = nxt_vlist_node.nxt_;
            vlistnode_fstream_.write(cur_vlist_node, cur_vlist_ptr);
            vlistnode_fstream_.free(nxt_vlist_ptr);
            // now check if nxt_body_node and nxt_head_node need to be erased or merged.
            if(cur_vlist_ptr == nxt_body_node.vlist_ptr_ && cur_vlist_node.nxt_.isnull()) {
              // this body node should be freed.
              vlistnode_fstream_.free(cur_vlist_ptr);
              cur_body_node.nxt_ = nxt_body_node.nxt_;
              bodynode_fstream_.write(cur_body_node, cur_body_ptr);
              bodynode_fstream_.free(nxt_body_ptr);
              if(cur_body_node.nxt_.isnull())
                nxt_head_node.high_key_ = cur_body_node.key_; // still valid when body_len = 0
              --nxt_head_node.body_len_;
              headnode_fstream_.write(nxt_head_node, nxt_head_ptr);
              if(nxt_head_node.body_len_ < degree / 2) {
                // merge if related head_nodes are small enough.
                bool is_left_valid = false, is_right_valid = false;
                if(cur_head_ptr != begin_head_ptr_) {
                  // left_ptr = cur_head_ptr valid
                  is_left_valid = true;
                  if(cur_head_node.body_len_ + nxt_head_node.body_len_ < degree) {
                    merge(cur_head_ptr, cur_head_node, nxt_head_ptr, nxt_head_node);
                    return;
                  }
                }
                HeadPtr right_ptr = nxt_head_node.nxt_;
                HeadNode right_node;
                if(!right_ptr.isnull()) {
                  // right_ptr = nxt_head_node.nxt_ valid
                  is_right_valid = true;
                  headnode_fstream_.read(right_node, right_ptr);
                  if(right_node.body_len_ + nxt_head_node.body_len_ > degree) {
                    merge(nxt_head_ptr, nxt_head_node, right_ptr, right_node);
                    return;
                  }
                }
                if(is_left_valid && is_right_valid) {
                  if(cur_head_node.body_len_ < right_node.body_len_)
                    average(cur_head_ptr, cur_head_node, nxt_head_ptr, nxt_head_node);
                  else average(nxt_head_ptr, nxt_head_node, right_ptr, right_node);
                  return;
                }
                if(is_left_valid) {
                  average(cur_head_ptr, cur_head_node, nxt_head_ptr, nxt_head_node);
                  return;
                }
                if(is_right_valid) {
                  average(nxt_head_ptr, nxt_head_node, right_ptr, right_node);
                  return;
                }
                // the only head node here.
                // then never care! It's accepted.
                return;
              }
              return; // redundant
            }
            return; // important
          }
        }
        // value too large, not found
        return;
      }
    }
    // code can't reach here, for key <= high_key.
  }
  // key is larger than any key existing.
  // not found.
}

template<class KeyType, class ValueType, size_t degree>
std::vector<ValueType> BlockList<KeyType, ValueType, degree>::operator[](const KeyType &key) {
  std::vector<ValueType> res;
    if(begin_head_ptr_.isnull())
    return res; // empty
  HeadPtr cur_head_ptr = begin_head_ptr_, nxt_head_ptr;
  HeadNode cur_head_node, nxt_head_node;
  headnode_fstream_.read(cur_head_node, cur_head_ptr); // must exist as the empty begin node
  nxt_head_ptr = cur_head_node.nxt_;
  while(!nxt_head_ptr.isnull()) {
    headnode_fstream_.read(nxt_head_node, nxt_head_ptr);
    if(key > nxt_head_node.high_key_) {
      cur_head_node = nxt_head_node;
      cur_head_ptr = nxt_head_ptr;
      nxt_head_ptr = cur_head_node.nxt_;
      continue;
    }
    // search in nxt_head
    BodyPtr cur_body_ptr = nxt_head_node.body_ptr_, nxt_body_ptr;
    BodyNode cur_body_node, nxt_body_node;
    bodynode_fstream_.read(cur_body_node, cur_body_ptr); // must exist as an empty begin node
    nxt_body_ptr = cur_body_node.nxt_;
    while(!nxt_body_ptr.isnull()) {
      bodynode_fstream_.read(nxt_body_node, nxt_body_ptr);
      if(key > nxt_body_node.key_) {
        cur_body_ptr = nxt_body_ptr;
        cur_body_node = nxt_body_node;
        nxt_body_ptr = cur_body_node.nxt_;
        continue;
      }
      if(key < nxt_body_node.key_) {
        // key not found.
        return res;
      }
      if(key == nxt_body_node.key_) {
        // key found in nxt_body_node.
        VListPtr cur_vlist_ptr = nxt_body_node.vlist_ptr_, nxt_vlist_ptr;
        VListNode cur_vlist_node, nxt_vlist_node;
        vlistnode_fstream_.read(cur_vlist_node, cur_vlist_ptr); // must exist as an empty begin node
        nxt_vlist_ptr = cur_vlist_node.nxt_;
        while(!nxt_vlist_ptr.isnull()) {
          vlistnode_fstream_.read(nxt_vlist_node, nxt_vlist_ptr);
          res.push_back(nxt_vlist_node.value_);
          cur_vlist_ptr = nxt_vlist_ptr;
          cur_vlist_node = nxt_vlist_node;
          nxt_vlist_ptr = cur_vlist_node.nxt_;
        }
        return res;
      }
    }
    // code can't reach here, for key <= high_key.
  }
  // key is larger than any key existing.
  // not found.
  return res;
}

template<class KeyType, class ValueType, size_t degree>
void BlockList<KeyType, ValueType, degree>::split(const HeadPtr &head_ptr, HeadNode &head_node) {
  int left_len = head_node.body_len_ / 2, right_len = head_node.body_len_ - left_len;
  BodyPtr cur_body_ptr = head_node.body_ptr_, nxt_body_ptr;
  BodyNode cur_body_node, nxt_body_node;
  bodynode_fstream_.read(cur_body_node, cur_body_ptr);
  nxt_body_ptr = cur_body_node.nxt_;
  bodynode_fstream_.read(nxt_body_node, nxt_body_ptr);
  for(int i = 0; i < left_len; i++) {
    cur_body_ptr = nxt_body_ptr;
    cur_body_node = nxt_body_node;
    nxt_body_ptr = cur_body_node.nxt_;
    bodynode_fstream_.read(nxt_body_node, nxt_body_ptr); // should always be valid if degree is big enough.
  }
  // shift nxt_node_ptr to another head node.
  BodyPtr new_body_begin = bodynode_fstream_.allocate({KeyType(), nxt_body_ptr, VListPtr()});
  HeadPtr new_right_head_ptr =
    headnode_fstream_.allocate({head_node.high_key_, head_node.nxt_, new_body_begin, right_len});
  cur_body_node.nxt_.setnull();
  bodynode_fstream_.write(cur_body_node, cur_body_ptr);
  head_node.nxt_ = new_right_head_ptr;
  head_node.high_key_ = cur_body_node.key_;
  head_node.body_len_ = left_len;
  headnode_fstream_.write(head_node, head_ptr);
}

template<class KeyType, class ValueType, size_t degree>
void BlockList<KeyType, ValueType, degree>::merge(
  const HeadPtr &left_ptr, HeadNode &left_node, const HeadPtr &right_ptr, const HeadNode &right_node) {

  // find left_node's last body node
  BodyPtr cur_body_ptr = left_node.body_ptr_, nxt_body_ptr;
  BodyNode cur_body_node, nxt_body_node;
  bodynode_fstream_.read(cur_body_node, cur_body_ptr);
  nxt_body_ptr = cur_body_node.nxt_;
  for(int i = 0; i < left_node.body_len_; i++) {
    bodynode_fstream_.read(nxt_body_node, nxt_body_ptr);
    cur_body_ptr = nxt_body_ptr;
    cur_body_node = nxt_body_node;
    nxt_body_ptr = cur_body_node.nxt_;
  }
  // assert(nxt_body_ptr.isnull());

  BodyNode right_begin_body_node;
  bodynode_fstream_.read(right_begin_body_node, right_node.body_ptr_);

  cur_body_node.nxt_ = right_begin_body_node.nxt_;
  bodynode_fstream_.write(cur_body_node, cur_body_ptr);
  bodynode_fstream_.free(right_node.body_ptr_);

  left_node.body_len_ += right_node.body_len_;
  left_node.high_key_ = right_node.high_key_;
  left_node.nxt_ = right_node.nxt_;
  headnode_fstream_.write(left_node, left_ptr);
  headnode_fstream_.free(right_ptr);
}

// sorry, it might be a little simple and brute.
template<class KeyType, class ValueType, size_t degree>
void BlockList<KeyType, ValueType, degree>::average(
  const HeadPtr &left_ptr, HeadNode &left_node, const HeadPtr &right_ptr, const HeadNode &right_node) {
  merge(left_ptr, left_node, right_ptr, right_node);
  // left_node.size may be over degree. But we'll split it right now.
  // Maybe it will cause some problem in concurrency implement.
  split(left_ptr, left_node);
}

} // namespace StarryPurple