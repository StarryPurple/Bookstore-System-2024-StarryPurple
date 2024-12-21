#include "blinktree.h"
#include <iostream>
#include <cstring>
using namespace std;
using StarryPurple::BLinkTree;


using KeyType = char [64];
using ValueType = int;
BLinkTree<KeyType, ValueType, 1 << 4, 100> multimap;

void insert(const string &key_str, const ValueType value) {
  KeyType key;
  strcpy(key, key_str.c_str());
  multimap.insert(key, value);
}

void find(const string &key_str) {
  KeyType key;
  strcpy(key, key_str.c_str());
  auto list = multimap.find(key);
  for(auto value: list)
    cout << value << " ";
  cout << endl;
}

void erase(const string &key_str, const ValueType value) {
  KeyType key;
  strcpy(key, key_str.c_str());
  multimap.erase(key, value);
}

int main() {
  string map_filename = "./data/test_map.bsdat";
  string key_filename = "./data/test_key.bsdat";
  string vlist_filename = "./data/test_vlist.bsdat";
  KeyType key; string key_str;
  multimap.open(map_filename, key_filename, vlist_filename);
  insert("FlowersForAlgernon", 1966);
  insert("CppPrimer", 2012);
  insert("Dune", 2021);
  insert("CppPrimer", 2001);
  find("CppPrimer");
  find("Java");
  erase("Dune", 2021);
  find("Dune");
  return 0;
}
/*
8
insert FlowersForAlgernon 1966
insert CppPrimer 2012
insert Dune 2021
insert CppPrimer 2001
find CppPrimer
find Java
delete Dune 2021
find Dune
*/