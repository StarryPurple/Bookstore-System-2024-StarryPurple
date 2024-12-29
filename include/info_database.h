#ifndef INFO_DATABASE_H
#define INFO_DATABASE_H

#include "infotypes.h"

#include <set>
#include <string>

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


class LoggedUsrType {
  friend UserStack;
  friend BookManager;
private:
  UserInfoType user_id;
  UserPrivilege privilege;
  ISBNType ISBN_selected{};
  bool has_selected_book = false;
public:
  LoggedUsrType() = default;
  LoggedUsrType(const UserType &user);
};

class UserStack {
  friend UserManager; // for active user
  friend BookManager; // for active user
  friend LogManager; // for actibe user, log commit
private:
  bool is_open = false;
  StarryPurple::Fstack<LoggedUsrType, cMaxFlowSize> u_stack;
  std::set<UserInfoType> logged_set;
  // The password infomation may be not right.
  LoggedUsrType &active_user();
  const UserPrivilege active_privilege();
  void open(const std::string &prefix);
  void close();
  void user_login(const UserType &user);
  void user_logout();
  void user_select_book(const ISBNType &ISBN);
  bool empty() const;
public:
  UserStack() = default;
  ~UserStack();
};

class UserDatabase {
  friend UserManager; // command "useradd" "register" "delete"
private:
  bool is_open = false;
  StarryPurple::Fmultimap<UserInfoType, UserType, 30, cMaxFlowSize> user_id_map;
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
  StarryPurple::Fmultimap<size_t, BookType, 30, cMaxFlowSize> book_map; // maps 0 to all books
  StarryPurple::Fmultimap<ISBNType, BookType, 30, cMaxFlowSize> ISBN_map;
  StarryPurple::Fmultimap<BookInfoType, BookType, 30, cMaxFlowSize>
    bookname_map, author_map, keyword_map;
  bool is_open = false;
  void open(const std::string &prefix);
  void close();
  static std::vector<BookInfoType> keyword_splitter(const BookInfoType &keyword_list);
  void book_register(const BookType &book);
  // modify list: [ISBN, bookname, author, keyword_list, price, storage]
  void book_modify_info(const BookType &old_book, BookType &modified_book, bool is_modified[6]);
  void book_change_storage(const BookType &book, const QuantityType &quantity); // quantity can be negative
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
  StarryPurple::Fstream<size_t, InfoType, 10> log_info;
  StarryPurple::Fmultimap<size_t, LogType, 30, cMaxFlowSize> all_log_id_map; // all logs
  StarryPurple::Fmultimap<size_t, LogType, 30, cMaxFlowSize> finance_log_id_map;
  StarryPurple::Fmultimap<size_t, LogType, 30, cMaxFlowSize> employee_work_log_id_map;
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