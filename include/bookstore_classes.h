#ifndef BOOKSTORE_CLASSES_H
#define BOOKSTORE_CLASSES_H

#include "utilities.h"

namespace BookStore {

using LogCountType = int;
constexpr LogCountType cMaxStorage = 100010;
template<int length> class cstr;
class UserPrivilege;
class UserType;
class BookType;
class UserManager;
class BookManager;
class LogManager;
class CommandManager;
class LogStack;
class UserDatabase;
class BookDatabase;
class LogDatabase;

using ISBNType = cstr<21>;
using BookInfoType = cstr<61>;
using UserInfoType = cstr<61>;
using PasswordType = cstr<31>;
using PriceType = double;
using QuantityType = long long;
using UserPtr = StarryPurple::Fpointer<cMaxStorage + 1>;
using BookPtr = StarryPurple::Fpointer<cMaxStorage + 2>;

template<int length>
class cstr {
private:
  char storage[length + 1];
public:
  cstr() = default;
  ~cstr() = default;
  cstr(const std::string &str);
  cstr(const cstr &other);
  bool operator==(const cstr &other) const;
  bool operator!=(const cstr &other) const;
  bool operator<=(const cstr &other) const;
  bool operator>=(const cstr &other) const;
  bool operator<(const cstr &other) const;
  bool operator>(const cstr &other) const;
};

class UserPrivilege {
private:
  int pri;
public:
  UserPrivilege();
  UserPrivilege(int privilege);
  UserPrivilege(const UserPrivilege &other);
  bool operator==(const UserPrivilege &other) const;
  bool operator<=(const UserPrivilege &other) const;
  bool operator>=(const UserPrivilege &other) const;
  bool operator!=(const UserPrivilege &other) const = delete;
  bool operator<(const UserPrivilege &other) const = delete;
  bool operator>(const UserPrivilege &other) const = delete;
};

class BookType {
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
private:
  UserInfoType ID, username, password;
  const UserPrivilege privilege;
public:
  UserType() = default;
  ~UserType() = default;
};

class LogStack {
  friend UserManager; // command "login" "logout"
private:
  struct LoggedUsrType {
    UserType user;
    BookType book_selected;
    bool has_selected_book = false;
  };
  StarryPurple::Fstack<LoggedUsrType, cMaxStorage> usr_stack;
  void user_login(const UserType &user);
  void user_logout();
public:
  LogStack() = default;
  ~LogStack() = default;
  LoggedUsrType &active_usr();
  UserPrivilege &active_privilege();
};

class UserDatabase {
  friend UserManager; // command "useradd" "register" "delete"
private:
  StarryPurple::Fmultimap<UserInfoType, UserPtr, 60, cMaxStorage> ID_map;
  void user_register(const UserType &user);
  void user_unregister(const UserInfoType &userID); // command "delete [userID]"
public:
  UserDatabase() = default;
  ~UserDatabase() = default;
};

class BookDatabase {
  friend BookManager; // command "select"
private:
  StarryPurple::Fmultimap<ISBNType, BookPtr, 60, cMaxStorage> ISBN_map;
  StarryPurple::Fmultimap<BookInfoType, BookPtr, 60, cMaxStorage>
    bookname_map, author_map, keyword_map;
public:
  BookDatabase() = default;
  ~BookDatabase() = default;
};

class LogDatabase {
public:
  LogDatabase() = default;
  ~LogDatabase() = default;
};

// user information database is managed here.
class UserManager {
  friend UserType;
public:
  UserManager() = default;
  ~UserManager() = default;
  void login(const UserInfoType &userID, const UserInfoType &password); // command "su [userID] [password]"
  void login(const UserInfoType &userID); // command "su [userID]"
  void user_register(const UserType &user); // command "register", "useradd"
  void change_password(
    const UserInfoType &userID,
    const UserInfoType &cur_pwd, const UserInfoType &new_pwd); // command "pwd [userID] [currentPassword] [newPassword]"
  void change_password(
    const UserInfoType &userID,
    const UserInfoType &new_pwd); // command "pwd [userID] [newPassword]"
  void logout(); // command "logout"
  void user_unregister(const UserInfoType &userID); // command "delete"
  void restock(const QuantityType &quantity, const PriceType &total_cost); // command "import"
};

// book information database is managed here.
class BookManager {
  friend BookType;
  friend UserManager; // book database is needed in "select"
public:
  BookManager() = default;
  ~BookManager() = default;
  void select(const ISBNType &ISBN); // command "select"
  void list_ISBN(const ISBNType &ISBN); // command "show -ISBN=[ISBN]"
  void list_bookname(const BookInfoType &bookname); // command "show -name=[bookname]"
  void list_author(const BookInfoType &author); // command "show -author=[author]"
  void list_keyword(const BookInfoType &keyword); // command "show -keyword=[keyword]"
  void sellout(const BookInfoType &ISBN, const QuantityType &quantity); // command "buy "
  void modify(
    const ISBNType &ISBN, const BookInfoType &bookname, const BookInfoType &author,
    const BookInfoType &keyword_list, const PriceType &price,
    bool is_param_given[5]);// command "modify"
};

class LogManager {
  friend BookType;
  friend UserType;
  struct InfoType;
  struct FinanceLogType;
private:
  struct InfoType {
    LogCountType log_count, deals_log_count;
    PriceType total_income;
  };
  struct FinanceLogType {
    PriceType income, expenditure;
  };
  StarryPurple::Fstream<FinanceLogType, InfoType, cMaxStorage> finance_log;
public:
  LogManager() = default;
  ~LogManager() = default;
  void show_deal_history(const LogCountType &count); // command "show finance [count]"
  void show_deal_history(); // special command "show finance"
  // TODO: Implement them later.
  // void report_finance(); // special command "report finance"
  // void report_employee(); // special command "report employee"
  // void report_history(); // special command "log"
};

class CommandManager {
  friend UserDatabase;
  friend BookDatabase;
  friend LogDatabase;
  friend UserManager;
  friend BookManager;
  friend LogManager;
  std::string command_reader();
  void quit(); // command "quit" "exit"
public:
  CommandManager() = default;
  ~CommandManager() = default;
  void command_list_reader();
};

} // namespace BookStore

#include "bookstore_classes.tpp"

#endif // BOOKSTORE_CLASSES_H