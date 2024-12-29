#include <iostream>
#include <cstring>
// #include "headers.h"
#include "command_manager.h"

class Other {
public:
  Other() = default;
};

int main() {
  // freopen("input.txt", "r", stdin); freopen("output.txt", "w", stdout);
  BookStore::CommandManager command_manager;
  command_manager.command_list_reader("../data", "Test");
  // fclose(stdin); fclose(stdout);
  return 0;
}