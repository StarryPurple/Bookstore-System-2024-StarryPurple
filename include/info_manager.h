#ifndef INFO_MANAGER_H
#define INFO_MANAGER_H

#include "infotypes.h"
#include "info_database.h"

namespace BookStore {

// friend classes

class CommandManager;

// new classes

class UserManager;
class BookManager;
class LogManager;

// user information database is managed here.
class UserManager {
  friend CommandManager;
private:
  UserStack user_stack;
  UserDatabase user_database;
  bool is_running = false;
  void open(const filenameType &prefix);
  void close();
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
  bool is_userid_occupied(const UserInfoType &userID);
public:
  UserManager() = default;
  ~UserManager();
};

// book information database is managed here.
class BookManager {
  friend UserManager; // book database is needed in "select"
  friend CommandManager;
private:
  BookDatabase book_database;
  bool is_running = false;
  void open(const filenameType &prefix);
  void close();
  void select_book(const ISBNType &ISBN); // command "select"
  void list_all(); // command "show" with no augments
  void list_ISBN(const ISBNType &ISBN); // command "show -ISBN=[ISBN]"
  void list_bookname(const BookInfoType &bookname); // command "show -name="[bookname]""
  void list_author(const BookInfoType &author); // command "show -author="[author]""
  void list_keyword(const BookInfoType &keyword); // command "show -keyword="[keyword]""
  void restock(const QuantityType &quantity, const PriceType &total_cost); // command "import"
  void sellout(const ISBNType &ISBN, const QuantityType &quantity); // command "buy"
  // command "modify"
  // whether other augments are given is determined by whether it's an empty ConstString.
  void modify_book(
    const ISBNType &ISBN, const BookInfoType &bookname, const BookInfoType &author,
    const BookInfoType &keyword_list, const PriceType &price, bool is_price_given);
public:
  BookManager() = default;
  ~BookManager();
};

class LogManager {
  friend CommandManager;
private:
  LogDatabase log_database;
  bool is_running = false;
  void open(const filenameType &prefix);
  void close();
  void show_deal_history(const LogCountType &count); // command "show finance [count]"
  void show_deal_history(); // special command "show finance"
  // TODO: Implement them later.
  void report_finance_history(); // special command "report finance"
  void report_employee_working_history(); // special command "report employee"
  void report_system_history(); // special command "log"
public:
  LogManager() = default;
  ~LogManager();
};


}

#endif // INFO_MANAGER_H