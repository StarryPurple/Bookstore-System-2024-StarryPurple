#ifndef BOOKSTORE_CLASSES_H
#define BOOKSTORE_CLASSES_H

#include "utilities.h"
#include "validator.h"

#include <regex>

namespace BookStore {

using StarryPurple::filenameType;
using StarryPurple::expect;

using LogCountType = int;
constexpr LogCountType cMaxStorage = 100010;
template<int length> class ConstStr;
class UserPrivilege;
class UserType;
class BookType;
class UserManager;
class BookManager;
class LogManager;
class CommandManager;
class UserStack;
class UserDatabase;
class BookDatabase;
class LogDatabase;

using ISBNType = ConstStr<21>;
using BookInfoType = ConstStr<61>;
using UserInfoType = ConstStr<61>;
using PasswordType = ConstStr<31>;
using PriceType = double;
using QuantityType = long long;
using UserPtr = StarryPurple::Fpointer<cMaxStorage + 1>;
using BookPtr = StarryPurple::Fpointer<cMaxStorage + 2>;

template<int capacity>
class ConstStr {
private:
  char storage[capacity + 1];
  size_t len;
public:
  ConstStr();
  ~ConstStr() = default;
  ConstStr(const std::string &str);
  ConstStr(const ConstStr &other);
  bool operator==(const ConstStr &other) const;
  bool operator!=(const ConstStr &other) const;
  bool empty() const;
};

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

class CommandManager {
  using ArglistType = std::vector<std::string>;
private:
  const std::string
    ascii_no_quotaton_alphabet =
      "!#$%&()*+,-./"
       "0123456789:;<=>?"
       "@ABCDEFGHIJKLMNO"
       "PQRSTUVWXYZ[\\]^_"
       "`abcdefghijklmno"
       "pqrstuvwxyz{|}~", // [BookName], [Author], [Keyword]
    ascii_alphabet =
      "!\"#$%&'()*+,-./"
       "0123456789:;<=>?"
       "@ABCDEFGHIJKLMNO"
       "PQRSTUVWXYZ[\\]^_"
       "`abcdefghijklmno"
       "pqrstuvwxyz{|}~", // [Username], [ISBN]
    digit_alpha_underline_alphabet =
      "_0123456789"
      "ABCDEFGHIJKLMNO"
      "PQRSTUVWXYZ"
      "abcdefghijklmno"
      "pqrstuvwxyz", // [UserID], [Password], [CurrentPassword], [NewPassword]
    user_priviledge_alphabet = "137", // [Privilege]
    digit_alphabet = "0123456789", // [Quantity], [Count]
    digit_with_dot_alphabet = ".0123456789"; // [Price] [TotalCost]
  const std::regex
    ISBN_aug_regex{"^-ISBN=([\\x20-\\x7E]+)$"},
    bookname_aug_regex{"^-name=\"([\\x20-\\x7E]+)\"$"}, // '\"' should be excluded before it is used
    author_aug_regex{"^-author=\"([\\x20-\\x7E]+)\"$"},
    keyword_aug_regex{"^-keyword=\"([\\x20-\\x7E]+)\"$"},
    price_aug_regex{"^-price=([\\x20-\\x7E]+)$"};
  UserManager user_manager;
  BookManager book_manager;
  LogManager log_manager;

  ArglistType command_splitter(const std::string &command);

  // Only checks if command is grammatically correct.
  // Whether the params fits other requirements are confirmed by other managers.

  void command_login(const ArglistType &argv); // command "su"
  void command_logout(const ArglistType &argv); // command "logout"
  void command_user_register(const ArglistType &argv); // command "register"
  void command_change_password(const ArglistType &argv); // command "passwd"
  void command_user_add(const ArglistType &argv); // command "useradd"
  void command_user_unregister(const ArglistType &argv); // command "delete"
  void command_list_book(const ArglistType &argv); // command "show", not "show finance"
  void command_show_finance(const ArglistType &argv); // command "show finance"
  void command_sellout(const ArglistType &argv); // command "buy"
  void command_select_book(const ArglistType &argv); // command "select"
  void command_modify_book(const ArglistType &argv); // command "modify"
  void command_restock(const ArglistType &argv); // command "import"
  void command_show_log(const ArglistType &argv); // command "log"
  void command_show_report(const ArglistType &argv); // command "report finance" "report employee"
  bool is_running = false;
public:
  CommandManager() = default;
  ~CommandManager();
  void open(const filenameType &prefix);
  void close();
  void command_list_reader();
};


} // namespace BookStore

#include "bookstore_classes.tpp"

#endif // BOOKSTORE_CLASSES_H