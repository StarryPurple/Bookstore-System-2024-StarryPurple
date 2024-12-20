#include "BookstoreExceptions.h"

namespace StarryPurple {

FileExceptions::FileExceptions(const std::string& exception_info) {
  exception_info_ += exception_info;
}

const char *FileExceptions::what() const noexcept {
  return exception_info_.c_str();
}

BLinkTreeExceptions::BLinkTreeExceptions(const std::string &exception_info) {
  exception_info_ += exception_info;
}

const char *BLinkTreeExceptions::what() const noexcept {
  return exception_info_.c_str();
}


} // namespace StarryPurple