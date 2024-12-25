#include <iostream>
#include <cstring>
#include "headers.h"
using namespace std;
using StarryPurple::Fstack;

int main() {
  Fstack<int, 100> stack;
  stack.open("test");
  stack.push(1);
  stack.push(2);
  stack.push(5);
  stack.push(-2);
  stack.push(0);
  stack.pop();
  stack.close();
  stack.open("test");
  while(!stack.empty()) {
    cout << stack.back() << " ";
    stack.pop();
  }
  return 0;
}