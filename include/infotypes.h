#ifndef INFO_TYPES_H
#define INFO_TYPES_H

#include "utilities.h"

namespace BookStore {

// friend classes

class UserStack;
class UserDatabase;
class BookDatabase;
class LogDatabase;
class UserManager;
class BookManager;
class LogManager;

// new classes

using StarryPurple::filenameType;
using StarryPurple::ConstStr;

class UserPrivilege;
class UserType;
class BookType;
class FinanceLogType;

using LogCountType = int;
constexpr LogCountType cMaxStorage = 100010;
using ISBNType = ConstStr<21>;
using BookInfoType = ConstStr<61>;
using UserInfoType = ConstStr<61>;
using PasswordType = ConstStr<31>;
using PriceType = double;
using QuantityType = long long;
using UserPtr = StarryPurple::Fpointer<cMaxStorage + 1>;
using BookPtr = StarryPurple::Fpointer<cMaxStorage + 2>;


class UserPrivilege {
private:
  const bool pri_1 = false, pri_2 = false, pri_3 = false;
public:
  UserPrivilege();
  explicit UserPrivilege(int privilege);
  UserPrivilege(const UserPrivilege &other);
  bool operator==(const UserPrivilege &other) const;
  bool operator<=(const UserPrivilege &other) const;
  bool operator>=(const UserPrivilege &other) const;
  bool operator!=(const UserPrivilege &other) const = delete;
  bool operator<(const UserPrivilege &other) const = delete;
  bool operator>(const UserPrivilege &other) const = delete;
};

class BookType {
  friend BookDatabase;
  friend BookManager; // needed?
private:
  ISBNType ISBN;
  BookInfoType bookname, author, keyword_list;
  PriceType price;
  QuantityType storage;
public:
  BookType() = default;
  ~BookType() = default;
};


class UserType {
  friend UserStack;
  friend UserDatabase;
  friend UserManager; // needed?
private:
  UserInfoType ID, username, password;
  const UserPrivilege privilege;
public:
  UserType() = default;
  ~UserType() = default;
  UserType(
    const UserInfoType &userID, const UserInfoType &passwd,
    int user_privilege, const UserInfoType &usrname);
};


class FinanceLogType {
private:
  PriceType income, expenditure;
};

} // namespace BookStore

#endif // INFO_TYPES_H