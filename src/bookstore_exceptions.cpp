#include "bookstore_exceptions.h"


StarryPurple::FileExceptions::FileExceptions(const std::string& exception_info) {
  exception_info_ += exception_info;
}

const char *StarryPurple::FileExceptions::what() const noexcept {
  return exception_info_.c_str();
}

StarryPurple::UtilityExceptions::UtilityExceptions(const std::string &exception_info) {
  exception_info_ += exception_info;
}

const char *StarryPurple::UtilityExceptions::what() const noexcept {
  return exception_info_.c_str();
}

const char *StarryPurple::ValidatorException::what() const noexcept {
  return "Validation failed.";
}
