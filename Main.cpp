#include <iostream>
#include <cstring>
#include "headers.h"

int main() {
  std::string str = "-author=\"Jane Eyre\"";
  std::regex author_regex{"^-author=\"([\\x20-\\x7E]+)\"$"};
  std::smatch match;
  if(std::regex_search(str, match, author_regex)) {
    std::cout << match.size() << std::endl;
    std::cout << match[1] << std::endl;
  } else std::cout << "Nod founda" << std::endl;
  return 0;
}