#ifndef BOOKSTORE_CLASSES_H
#define BOOKSTORE_CLASSES_H

#include "utilities.h"

namespace BookStore {

constexpr int cMaxStorage = 100010;
template<int length> class cstr;
class UsrPrivilege;
class UsrType;
class BookType;
class UsrManager;
class BookManager;
class LogManager;
class CommandManager;

using ISBNType = cstr<21>;
using BookInfoType = cstr<61>;
using UsrInfoType = cstr<61>;
using PwdType = cstr<31>;

template<int length>
class cstr {
private:
  char storage[length + 1];
public:
  cstr();
  cstr(const std::string &str);
  cstr(const cstr &other);
  bool operator==(const cstr &other) const;
  bool operator!=(const cstr &other) const;
  bool operator<=(const cstr &other) const;
  bool operator>=(const cstr &other) const;
  bool operator<(const cstr &other) const;
  bool operator>(const cstr &other) const;
};

class UsrPrivilege {
private:
  int pri;
public:
  UsrPrivilege(int privilege);
  UsrPrivilege(const UsrPrivilege &other);
  bool operator==(const UsrPrivilege &other) const;
  bool operator<=(const UsrPrivilege &other) const;
  bool operator>=(const UsrPrivilege &other) const;
  bool operator!=(const UsrPrivilege &other) const = delete;
  bool operator<(const UsrPrivilege &other) const = delete;
  bool operator>(const UsrPrivilege &other) const = delete;
};

class BookType {
  friend BookManager;
  friend LogManager;
private:
  ISBNType ISBN;
  BookInfoType bookname, author, keyword_cstr;
  double price;
  long long storage;
};

class UsrType {
  friend UsrManager;
  friend LogManager;
private:
  UsrInfoType ID, username, password;
  const UsrPrivilege privilege;
};

class UsrManager {
  friend CommandManager;
  struct LoggedUsrType;
private:
  struct LoggedUsrType {
    UsrType usr;
    BookType book_selected;
    bool has_selected_book = false;
  };
};

class BookManager {
  friend CommandManager;

};

class LogManager {
  friend CommandManager;

};

class CommandManager {

};

} // namespace BookStore

#include "bookstore_classes.tpp"

#endif // BOOKSTORE_CLASSES_H