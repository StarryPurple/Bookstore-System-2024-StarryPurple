#ifndef INFO_DATABASE_H
#define INFO_DATABASE_H

#include "infotypes.h"

#include <unordered_set>

namespace BookStore {



// friend classes

class UserManager;
class BookManager;
class LogManager;

// new classes

class LoggedUsrType;
class UserStack;
class UserDatabase;
class BookDatabase;
class LogDatabase;

using UserPtr = StarryPurple::Fpointer<cMaxFlowSize + 1>; // accord to user database capacity
using BookPtr = StarryPurple::Fpointer<cMaxFlowSize + 2>; // accord to book database capacity
using LogPtr = StarryPurple::Fpointer<cMaxFlowSize + 3>; // accord to log database sapacity

class LoggedUsrType {
  friend UserStack;
  friend BookManager;
private:
  UserType user;
  BookType book_selected{};
  bool has_selected_book = false;
public:
  LoggedUsrType(const UserType &usr);
};

class UserStack {
  friend UserManager; // command "login" "logout"
  friend BookManager; // for active user
  friend LogManager; // log commit
private:
  bool is_open = false;
  StarryPurple::Fstack<LoggedUsrType, cMaxFlowSize> u_stack;
  std::unordered_set<UserInfoType> logged_set;
  // The password infomation may be not right.
  LoggedUsrType &active_usr();
  const UserPrivilege &active_privilege();
  void open(const std::string &prefix);
  void close();
  void user_login(const UserType &user);
  void user_logout();
  void user_select_book(const BookType &book);
  bool empty() const;
public:
  UserStack() = default;
  ~UserStack();
};

class UserDatabase {
  friend UserManager; // command "useradd" "register" "delete"
private:
  size_t user_number;
  bool is_open = false;
  StarryPurple::Fstream<UserType, size_t, cMaxFlowSize + 1> user_data;
  StarryPurple::Fmultimap<UserInfoType, UserPtr, 30, cMaxFlowSize> id_map;
  void open(const std::string &prefix);
  void close();
  void user_register(const UserType &user);
  void user_unregister(const UserInfoType &userID); // command "delete [userID]"
public:
  UserDatabase() = default;
  ~UserDatabase();
};

// book id is the order it's added.
class BookDatabase {
  friend BookManager; // command "select"
private:
  size_t book_number;
  StarryPurple::Fstream<BookType, size_t, cMaxFlowSize + 2> book_data;
  StarryPurple::Fmultimap<size_t, BookPtr, 30, cMaxFlowSize> ID_map;
  StarryPurple::Fmultimap<ISBNType, BookPtr, 30, cMaxFlowSize> ISBN_map;
  StarryPurple::Fmultimap<BookInfoType, BookPtr, 30, cMaxFlowSize>
    bookname_map, author_map, keyword_map;
  bool is_open = false;
  void open(const std::string &prefix);
  void close();
  std::vector<BookInfoType> keyword_splitter(const BookInfoType &keyword_list);
  void book_register(const BookType &book);
  // modify list: [ISBN, bookname, author, keyword_list, price]
  void book_modify_info(const BookPtr &book_ptr, const BookType &new_book, bool is_modified[5]);
public:
  BookDatabase() = default;
  ~BookDatabase();
};

class LogDatabase {
  friend LogManager;
  struct InfoType;
private:
  struct InfoType {
    LogCountType employee_work_log_count, finance_log_count, all_log_count;
    PriceType total_income, total_expenditure;
  };
  InfoType info;
  StarryPurple::Fstream<LogType, InfoType, cMaxFlowSize + 3> log_data;
  StarryPurple::Fmultimap<size_t, LogPtr, 30, cMaxFlowSize> all_log_id_map; // all logs
  StarryPurple::Fmultimap<size_t, LogPtr, 30, cMaxFlowSize> finance_log_id_map;
  StarryPurple::Fmultimap<size_t, LogPtr, 30, cMaxFlowSize> employee_work_log_id_map;
  bool is_open = false;
  // Common:
  //   record everyone's call for all commands:
  // Param 0x01:
  //   record an employee's / the bookstore keeper's call for commands:
  //   "useradd" "select" "modify" "import"
  // Param 0x10:
  //   record the income / expenditure change.
  // Param 0x11:
  //   record finance change and privileged user's specific actions.
  //
  // So log_level should be among {0, 1, 2, 3}.
  void add_log(
    const PriceType &income, const PriceType &expenditure,
    const LogDescriptionType &description, int log_level);

  void open(const std::string &prefix);
  void close();
public:
  LogDatabase() = default;
  ~LogDatabase();
};

}

#endif // INFO_DATABASE_H