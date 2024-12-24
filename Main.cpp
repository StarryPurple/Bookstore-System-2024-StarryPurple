#include "filestream.h"
#include "filestream.cpp"
#include "blocklist.h"
#include "blocklist.cpp"
#include "lrucache.h"
#include "lrucache.cpp"
#include "fmultimap.h"
#include "fmultimap.cpp"
#include <iostream>
#include <cstring>
using namespace std;
using StarryPurple::Fmultimap;
using StarryPurple::BlockList;

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
  std::string expose_str() const {
    return {val};
  }
private:
  char val[65];
};

template<>
struct std::hash<KeyType> {
  std::size_t operator()(const KeyType &key) const noexcept {
    return std::hash<std::string>()(key.expose_str());
  }
};

using ValueType = int;
// sqrt(1e5) ~ 316.2
// Oh I know. not all space reserved can be used without key space reallocate.
Fmultimap<KeyType, ValueType, 48, 180100> multimap;
// BlockList<KeyType, ValueType, 500> multimap;

void insert(const KeyType &key, const ValueType value) {
  multimap.insert(key, value);
}

void find(const KeyType &key) {
  auto list = multimap[key];
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
  // freopen("input.txt", "r", stdin);
  // freopen("output.txt", "w", stdout);
  string filename_suffix = "test";
  multimap.open(filename_suffix);
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
  // fclose(stdin); fclose(stdout);
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