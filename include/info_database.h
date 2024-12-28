#ifndef INFO_DATABASE_H
#define INFO_DATABASE_H

#include "infotypes.h"

namespace BookStore {



// friend classes

class UserManager;
class BookManager;
class LogManager;

// new classes

class UserStack;
class UserDatabase;
class BookDatabase;
class LogDatabase;

class UserStack {
  friend UserManager; // command "login" "logout"
private:
  struct LoggedUsrType {
    UserType user;
    BookType book_selected;
    bool has_selected_book = false;
  };
  bool is_open = false;
  StarryPurple::Fstack<LoggedUsrType, cMaxStorage> usr_stack;
  void user_login(const UserType &user);
  void open(const std::string &prefix);
  void close();
  void user_logout();
public:
  UserStack();
  ~UserStack();
  LoggedUsrType &active_usr();
  UserPrivilege &active_privilege();
};

class UserDatabase {
  friend UserManager; // command "useradd" "register" "delete"
private:
  bool is_open = false;
  StarryPurple::Fmultimap<UserInfoType, UserPtr, 60, cMaxStorage> ID_map;
  void open(const std::string &prefix);
  void close();
  void user_register(const UserType &user);
  void user_unregister(const UserInfoType &userID); // command "delete [userID]"
public:
  UserDatabase();
  ~UserDatabase();
};

class BookDatabase {
  friend BookManager; // command "select"
private:
  StarryPurple::Fmultimap<ISBNType, BookPtr, 60, cMaxStorage> ISBN_map;
  StarryPurple::Fmultimap<BookInfoType, BookPtr, 60, cMaxStorage>
    bookname_map, author_map, keyword_map;
  bool is_open = false;
  void open(const std::string &prefix);
  void close();
  void book_register(const BookType &book, bool is_param_given[5]);
public:
  BookDatabase();
  ~BookDatabase();
};

class LogDatabase {
  friend LogManager;
  struct InfoType;
private:
  struct InfoType {
    LogCountType log_count, deals_log_count;
    PriceType total_income;
  };
  StarryPurple::Fstream<FinanceLogType, InfoType, cMaxStorage> finance_log;
  bool is_open = false;
  void show_deal_history(const LogCountType &count); // command "show finance [count]"
  void show_deal_history(); // special command "show finance"
  // TODO: Implement them later.
  // void report_finance(); // special command "report finance"
  // void report_employee(); // special command "report employee"
  // void report_history(); // special command "log"
  void open(const std::string &prefix);
  void close();
public:
  LogDatabase();
  ~LogDatabase();
};

}

#endif // INFO_DATABASE_H