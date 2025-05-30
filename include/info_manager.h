#ifndef INFO_MANAGER_H
#define INFO_MANAGER_H

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
  void open(const std::string &prefix);
  void close();
  LogType login(const UserInfoType &userID, const PasswordType &password); // command "su [userID] [password]"
  LogType login(const UserInfoType &userID); // command "su [userID]"
  LogType user_register(const UserType &user); // command "register"
  LogType user_add(const UserType &user); // command "useradd"
  LogType change_password(
    const UserInfoType &userID,
    const PasswordType &cur_pwd, const PasswordType &new_pwd); // command "pwd [userID] [currentPassword] [newPassword]"
  LogType change_password(
    const UserInfoType &userID,
    const PasswordType &new_pwd); // command "pwd [userID] [newPassword]"
  LogType logout(); // command "logout"
  LogType user_unregister(const UserInfoType &userID); // command "delete"
public:
  UserManager() = default;
  ~UserManager();
};

// book information database is managed here.
class BookManager {
  friend CommandManager;
private:
  BookDatabase book_database;
  UserStack *user_stack_ptr;
  bool is_running = false;
  void open(const std::string &prefix);
  void close();
  void select_book(const ISBNType &ISBN); // command "select"
  void list_all(); // command "show" with no augments
  void list_ISBN(const ISBNType &ISBN); // command "show -ISBN=[ISBN]"
  void list_bookname(const BookInfoType &bookname); // command "show -name="[bookname]""
  void list_author(const BookInfoType &author); // command "show -author="[author]""
  void list_keyword(const BookInfoType &keyword); // command "show -keyword="[keyword]""
  LogType restock(const QuantityType &quantity, const PriceType &total_cost); // command "import"
  LogType sellout(const ISBNType &ISBN, const QuantityType &quantity); // command "buy"
  // command "modify"
  // whether other augments are given is determined by whether it's an empty ConstString.
  LogType modify_book(
    const ISBNType &ISBN, const BookInfoType &bookname, const BookInfoType &author,
    const BookInfoType &keyword_list, const PriceType &price, bool is_modified[5]);
public:
  BookManager() = default;
  ~BookManager();
};

class LogManager {
  friend UserManager; // for log_add
  friend BookManager; // for log_add
  friend CommandManager;
private:
  LogDatabase log_database;
  UserStack *user_stack_ptr;
  bool is_running = false;
  void open(const std::string &prefix);
  void close();
  // Common:
  //   record everyone's call for important commands:
  //   "su" "register" "useradd" "passwd" "logout" "delete" "buy" "import" "modify"
  //   Also contains #system-starton, "quit" "exit".
  //   Do not record logout caused by system shutdown and brute system-shutdown.
  // Param 0x01:
  //   record an employee's / the bookstore keeper's call for special commands:
  //   "useradd" "select" "modify" "import"
  // Param 0x10:
  //   record the income / expenditure change.
  //   "buy" "import"
  // Param 0x11:
  //   record finance change and privileged user's specific actions.
  //   "import"
  //
  // So log_level should be among {0, 1, 2, 3}.
  void add_log(
    const LogType &log, int log_level);

  void show_deal_history(const LogCountType &count); // command "show finance [count]"
  void show_deal_history(); // special command "show finance"
  void report_finance(); // special command "report finance"
  void report_employee(); // special command "report employee"
  void report_history(); // special command "log"
public:
  LogManager() = default;
  ~LogManager();
};


}

#endif // INFO_MANAGER_H