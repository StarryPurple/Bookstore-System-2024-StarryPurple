#include <iostream>
#include <cstring>
#include "headers.h"

int main() {
  BookStore::CommandManager command_manager;
  command_manager.command_list_reader();
  return 0;
}