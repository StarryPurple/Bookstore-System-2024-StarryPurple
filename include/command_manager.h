#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include "info_manager.h"

#include <regex>

namespace BookStore {

class CommandManager;

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

  void open(const filenameType &prefix);
  void close();

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
  // @prefix shouldn't have '/' in it. It's just a prefix for all data files.
  // @directory should end with '/', for example "./data/".
  // if not assigned, directory = "./", means the data will be stored in you current directory.
  void command_list_reader(const filenameType &prefix, const filenameType &directory = "./");
};
}

#endif // COMMAND_MANAGER_H