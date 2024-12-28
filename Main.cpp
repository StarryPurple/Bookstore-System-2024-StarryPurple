#include <iostream>
#include <cstring>
// #include "headers.h"
#include "command_manager.h"

int main() {
  // freopen("input.txt", "r", stdin); freopen("output.txt", "w", stdout);
  BookStore::CommandManager command_manager;
  command_manager.command_list_reader();
  // fclose(stdin); fclose(stdout);
  return 0;
}