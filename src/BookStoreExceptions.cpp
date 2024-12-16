#include "BookstoreExceptions.h"

namespace StarryPurple_Exceptions {

FileExceptions::FileExceptions(std::string exception_info) {
  exception_info_ += exception_info;
}

const char *FileExceptions::what() const {
  return exception_info_.c_str();
}

}