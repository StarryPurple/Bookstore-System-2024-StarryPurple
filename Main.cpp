#include <iostream>
#include <cstring>
// #include "headers.h"
#include "command_manager.h"

void Main() {
  // 19 files are used. Can I cut this number down?
  // freopen("input.txt", "r", stdin); freopen("output.txt", "w", stdout);
  BookStore::CommandManager command_manager;
  command_manager.command_list_reader("Test", "./");
  // fclose(stdin); fclose(stdout);
}

int main() {
  Main();
  return 0;
}