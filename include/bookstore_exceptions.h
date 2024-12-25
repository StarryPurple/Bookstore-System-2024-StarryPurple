#pragma once
#ifndef BOOKSTORE_EXCEPTIONS_H
#define BOOKSTORE_EXCEPTIONS_H

#include <stdexcept>
#include <iostream>

namespace StarryPurple {

class FileExceptions: std::exception {

public:
  FileExceptions() = delete;
  FileExceptions(const std::string& exception_info);
  ~FileExceptions() = default;
  const char *what() const noexcept override;

private:
  std::string exception_info_ = "File error: ";
};

class UtilityExceptions: std::exception {

public:
  UtilityExceptions() = delete;
  UtilityExceptions(const std::string& exception_info);
  ~UtilityExceptions() = default;
  const char *what() const noexcept override;

private:
  std::string exception_info_ = "Utility error: ";
};

class ValidatorException: std::exception {
  const char *what() const noexcept override;
};

} // namespace StarryPurple




#endif // BOOKSTORE_EXCEPTIONS_H