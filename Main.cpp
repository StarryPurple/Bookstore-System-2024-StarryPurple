#include <iostream>
#include <cstring>
#include "utilities.h"
#include "insomnia_multimap.h"

void multimap_test() {
  using KeyType = StarryPurple::ConstStr<64>;
  using ValueType = int;
  // StarryPurple::Fmultimap<KeyType, ValueType, 32, StarryPurple::cCapacity> fmultimap;
  Insomnia::BlinkTree<KeyType, ValueType, 32, 16000> fmultimap;
  fmultimap.open("tst");
  int n; std::cin >> n;
  std::string op, key_str;
  ValueType value;
  while(n--) {
    std::cin >> op;
    if(op == "insert") {
      std::cin >> key_str >> value;
      fmultimap.insert(KeyType(key_str), value);
    } else if(op == "find") {
      std::cin >> key_str;
      auto list = fmultimap[KeyType(key_str)];
      if(list.empty())
        std::cout << "null";
      else
        for(const auto &val: list)
          std::cout << val << " ";
      std::cout << std::endl;
    } else if(op == "delete") {
      std::cin >> key_str >> value;
      fmultimap.erase(KeyType(key_str), value);
    }
  }
  fmultimap.close();
}

/*
void Main() {
  // 19 files are used. Can I cut this number down?
  // freopen("input.txt", "r", stdin); freopen("output.txt", "w", stdout);
  BookStore::CommandManager command_manager;
  command_manager.command_list_reader("Test", "./");
  // fclose(stdin); fclose(stdout);
}
*/

int main() {
  multimap_test();
  /*
  try {
    multimap_test();
  } catch (StarryPurple::FileExceptions &ex) {
    std::cout << ex.what() << std::endl;
  }*/
  // Main();
  return 0;
}