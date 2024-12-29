#include <iostream>
#include <cstring>
// #include "headers.h"
#include "command_manager.h"

void mian() {
  std::vector<std::string> argv = {"su", "root", "sjtu"};
  assert(argv.size() == 3);
  StarryPurple::expect(argv.size()).toBeOneOf(2, 3);
}

void Main() {
  // freopen("input.txt", "r", stdin); freopen("output.txt", "w", stdout);
  BookStore::CommandManager command_manager;
  command_manager.command_list_reader("Test", "./");
  // fclose(stdin); fclose(stdout);
}

int main() {
  Main();
  return 0;
}