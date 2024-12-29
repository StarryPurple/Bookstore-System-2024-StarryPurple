#ifndef INFO_TYPES_H
#define INFO_TYPES_H

#include "utilities.h"

namespace BookStore {

using StarryPurple::ConstStr;
using StarryPurple::expect;

// friend classes

class LoggedUsrType;
class UserStack;
class UserDatabase;
class BookDatabase;
class LogDatabase;
class UserManager;
class BookManager;
class LogManager;

// new classes


class UserPrivilege;
class UserType;
class BookType;
class LogType;

using LogCountType = int;
constexpr LogCountType cMaxFlowSize = 100010;
using ISBNType = ConstStr<21>;
using BookInfoType = ConstStr<61>;
using UserInfoType = ConstStr<61>;
using PasswordType = ConstStr<31>;
using LogDescriptionType = ConstStr<501>;
using PriceType = double;
using QuantityType = long long;

using UserPtr = StarryPurple::Fpointer<cMaxFlowSize + 1>; // accord to user database capacity
using BookPtr = StarryPurple::Fpointer<cMaxFlowSize + 2>; // accord to book database capacity
using LogPtr = StarryPurple::Fpointer<cMaxFlowSize + 3>; // accord to log database sapacity

class UserPrivilege {
private:
  bool pri_1 = false, pri_2 = false, pri_3 = false;
public:
  UserPrivilege() = default;
  explicit UserPrivilege(int privilege);
  UserPrivilege(const UserPrivilege &other);
  bool operator==(const UserPrivilege &other) const;
  bool operator!=(const UserPrivilege &other) const;
  bool operator<=(const UserPrivilege &other) const;
  bool operator>=(const UserPrivilege &other) const;
};

class BookType {
  friend BookDatabase;
  friend BookManager; // needed?
private:
  ISBNType isbn{};
  BookInfoType bookname{}, author{}, keyword_list{};
  PriceType price = 0;
  QuantityType storage = 0;
public:
  BookType() = default;
  BookType(const ISBNType &ISBN);
  ~BookType() = default;
  void print() const;
  bool operator==(const BookType &other) const;
  bool operator!=(const BookType &other) const;
  bool operator<(const BookType &other) const;
  bool operator>(const BookType &other) const;
  bool operator<=(const BookType &other) const;
  bool operator>=(const BookType &other) const;
};


class UserType {
  friend LoggedUsrType;
  friend UserStack;
  friend UserDatabase;
  friend UserManager; // needed?
private:
  UserInfoType ID, username;
  PasswordType passwd;
  UserPrivilege privilege;
public:
  UserType() = default;
  UserType(
    const UserInfoType &userID, const PasswordType &password,
    int user_privilege, const UserInfoType &name);
  ~UserType() = default;
  bool operator==(const UserType &other) const;
  bool operator!=(const UserType &other) const;
  bool operator<(const UserType &other) const;
  bool operator>(const UserType &other) const;
  bool operator<=(const UserType &other) const;
  bool operator>=(const UserType &other) const;

};


class LogType {
  friend LogManager; // used in "show finance"
  friend LogDatabase;
private:
  size_t id;
  PriceType total_income, total_expenditure;
  LogDescriptionType log_description;
public:
  static size_t log_count;
  LogType() = default;
  LogType(
    const PriceType &history_income, const PriceType &history_expenditure,
    const LogDescriptionType &description);
  ~LogType() = default;
  bool operator==(const LogType &other) const;
  bool operator!=(const LogType &other) const;
  bool operator<(const LogType &other) const;
  bool operator>(const LogType &other) const;
  bool operator<=(const LogType &other) const;
  bool operator>=(const LogType &other) const;
};

} // namespace BookStore

#endif // INFO_TYPES_H