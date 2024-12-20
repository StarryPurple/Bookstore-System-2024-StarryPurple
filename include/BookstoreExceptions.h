/** BookstoreExceptions.h
 * Author: StarryPurple
 * Date: Since 2024.12.16
 *
 * The exceptions that may be used in the bookstore system.
 */

#pragma once
#ifndef BOOKSTORE_EXCEPTIONS_H
#define BOOKSTORE_EXCEPTIONS_H

#include <stdexcept>

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

class BLinkTreeExceptions: std::exception {

public:
  BLinkTreeExceptions() = delete;
  BLinkTreeExceptions(const std::string& exception_info);
  ~BLinkTreeExceptions() = default;
  const char *what() const noexcept override;

private:
  std::string exception_info_ = "B-link tree error: ";
};

} // namespace StarryPurple




#endif // BOOKSTORE_EXCEPTIONS_H