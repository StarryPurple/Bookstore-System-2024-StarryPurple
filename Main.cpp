#include "filestream.h"
#include "filestream.cpp"
#include "fmultimap.h"
#include "fmultimap.cpp"
#include <iostream>
#include <cstring>
using namespace std;
using StarryPurple::Fmultimap;

struct KeyType {
  KeyType() {
    for(int i = 0; i < 65; i++)
      val[i] = '\0';
  }

  KeyType(const KeyType &other) {
    for(int i = 0; i < 65; i++)
      val[i] = other.val[i];
  }

  KeyType(const string &str) {
    strcpy(val, str.c_str());
  }

  KeyType &operator=(const KeyType &other) {
    if(this == &other) return *this;
    for(int i = 0; i < 65; i++)
      val[i] = other.val[i];
    return *this;
  }

  KeyType &operator=(const string &str) {
    strcpy(val, str.c_str());
    return *this;
  }

  bool operator<(const KeyType &other) const {
    for(int i = 0; i < 65; i++) {
      if(val[i] == '\0' && other.val[i] == '\0')
        return false;
      if(val[i] != other.val[i])
        return val[i] < other.val[i];
    }
    return false;
  }

  bool operator>(const KeyType &other) const {
    for(int i = 0; i < 65; i++) {
      if(val[i] == '\0' && other.val[i] == '\0')
        return false;
      if(val[i] != other.val[i])
        return val[i] > other.val[i];
    }
    return false;
  }

  bool operator<=(const KeyType &other) const {
    return !(*this > other);
  }
  bool operator>=(const KeyType &other) const {
    return !(*this < other);
  }
  bool operator==(const KeyType &other) const {
    for(int i = 0; i < 65; i++) {
      if(val[i] == '\0' && other.val[i] == '\0')
        return true;
      if(val[i] != other.val[i])
        return false;
    }
    // throw runtime_error("Invalid comparision");
    return true;
  }
  bool operator!=(const KeyType &other) const {
    return !(*this == other);
  }
  friend ostream &operator<<(ostream &os, const KeyType &key) {
    os << key.val;
    return os;
  }
private:
  char val[65];
};

using ValueType = int;
Fmultimap<KeyType, ValueType, 48, 100010 / 48 + 20> multimap;

void insert(const KeyType &key, const ValueType value) {
  multimap.insert(key, value);
}

void find(const KeyType &key) {
  auto list = multimap.find(key);
  if(list.empty())
    cout << "null";
  else
    for(auto value: list)
      cout << value << " ";
  cout << endl;
}

void erase(const KeyType &key, const ValueType value) {
  multimap.erase(key, value);
}

int main() {
  string map_filename = "../data/test_map.bsdat";
  string vlist_filename = "../data/test_vlist.bsdat";
  multimap.open(map_filename, vlist_filename);
  int n; cin >> n;
  string oper, key_str;
  KeyType key;
  ValueType value;
  while(n--) {
    cin >> oper;
    if(oper == "insert") {
      cin >> key_str; key = key_str;
      cin >> value;
      insert(key, value);
    } else if(oper == "find") {
      cin >> key_str; key = key_str;
      find(key);
    } else if(oper == "delete") {
      cin >> key_str; key = key_str;
      cin >> value;
      erase(key, value);
    }
  }
  multimap.close();
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