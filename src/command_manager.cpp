#include "command_manager.h"

#include <vector>
#include <iostream>

using StarryPurple::expect;

void BookStore::CommandManager::command_login(const ArglistType &argv) {
  // "su [UserID] ([Password])?"
  expect(argv.size()).toBeOneOf(2, 3);
  expect(argv[1]).toBeConsistedOf(digit_alpha_underline_alphabet);
  if(argv.size() == 2)
    user_manager.login(UserInfoType(argv[1]));
  else {
    expect(argv[2]).toBeConsistedOf(digit_alpha_underline_alphabet);
    user_manager.login(UserInfoType(argv[1]), PasswordType(argv[2]));
  }
}

void BookStore::CommandManager::command_logout(const ArglistType &argv) {
  // “logout”
  expect(argv.size()).toBe(1);
  user_manager.logout();
}

void BookStore::CommandManager::command_user_register(const ArglistType &argv) {
  // "register [UserID] [Password] [Username]"
  expect(argv.size()).toBe(4);
  expect(argv[1]).toBeConsistedOf(digit_alpha_underline_alphabet);
  expect(argv[2]).toBeConsistedOf(digit_alpha_underline_alphabet);
  expect(argv[3]).toBeConsistedOf(ascii_alphabet);
  user_manager.user_register(
    UserType(
      UserInfoType(argv[1]), PasswordType(argv[2]),
      1, UserInfoType(argv[3])));
}

void BookStore::CommandManager::command_change_password(const ArglistType &argv) {
  // "passwd [UserID] ([CurrentPassword])? [NewPassword]"
  expect(argv.size()).toBeOneOf(3, 4);
  expect(argv[1]).toBeConsistedOf(digit_alpha_underline_alphabet);
  if(argv.size() == 4) {
    expect(argv[2]).toBeConsistedOf(digit_alpha_underline_alphabet);
    expect(argv[3]).toBeConsistedOf(digit_alpha_underline_alphabet);
    user_manager.change_password(
      UserInfoType(argv[1]), PasswordType(argv[2]), PasswordType(argv[3]));
  } else {
    expect(argv[2]).toBeConsistedOf(digit_alpha_underline_alphabet);
    user_manager.change_password(
      UserInfoType(argv[1]), PasswordType(argv[2]));
  }
}

void BookStore::CommandManager::command_user_add(const ArglistType &argv) {
  // "useradd [UserID] [Password] [Privilege] [Username]"
  expect(argv.size()).toBe(5);
  expect(argv[1]).toBeConsistedOf(digit_alpha_underline_alphabet);
  expect(argv[2]).toBeConsistedOf(digit_alpha_underline_alphabet);
  expect(argv[3]).toBeConsistedOf(user_priviledge_alphabet);
  expect(argv[4]).toBeConsistedOf(ascii_alphabet);
  int pri = std::stoi(argv[3]);
  expect(pri).toBeOneOf(1, 3, 7);
  user_manager.user_add(
    UserType(
      UserInfoType(argv[1]), PasswordType(argv[2]),
      pri, UserInfoType(argv[4])));
}

void BookStore::CommandManager::command_user_unregister(const ArglistType &argv) {
  // "delete [UserID]"
  expect(argv.size()).toBe(2);
  expect(argv[1]).toBeConsistedOf(digit_alpha_underline_alphabet);
  user_manager.user_unregister(UserInfoType(argv[1]));
}

void BookStore::CommandManager::command_list_book(const ArglistType &argv) {
  // "show (-ISBN=[ISBN] | -name="[BookName]" | -author="[Author]" | -keyword="[Keyword]")?"
  expect(argv.size()).toBeOneOf(1, 2);
  if(argv.size() == 1)
    book_manager.list_all();
  else {
    std::smatch match;
    if(std::regex_search(argv[1], match, ISBN_aug_regex)) {
      std::string ISBN = match[1];
      expect(ISBN).toBeConsistedOf(ascii_alphabet);
      expect(ISBN.empty()).toBe(false);
      book_manager.list_ISBN(ISBNType(ISBN));
    } else if(std::regex_search(argv[1], match, bookname_aug_regex)) {
      std::string bookname = match[1];
      expect(bookname).toBeConsistedOf(ascii_no_double_quotaton_alphabet);
      expect(bookname.empty()).toBe(false);
      book_manager.list_bookname(BookInfoType(bookname));
    } else if(std::regex_search(argv[1], match, author_aug_regex)) {
      std::string author = match[1];
      expect(author).toBeConsistedOf(ascii_no_double_quotaton_alphabet);
      expect(author.empty()).toBe(false);
      book_manager.list_author(BookInfoType(author));
    } else if(std::regex_search(argv[1], match, keyword_aug_regex)) {
      std::string keyword = match[1];
      expect(keyword).toBeConsistedOf(ascii_no_double_quotaton_alphabet);
      expect(keyword.empty()).toBe(false);
      book_manager.list_keyword(BookInfoType(keyword));
    } else throw StarryPurple::ValidatorException();
  }
}

void BookStore::CommandManager::command_show_finance(const ArglistType &argv) {
  // "show finance ([Count])?"
  expect(argv.size()).toBeOneOf(2, 3);
  if(argv.size() == 2)
    log_manager.show_deal_history();
  else {
    expect(argv[2]).toBeConsistedOf(digit_alphabet);
    LogCountType count = std::stoi(argv[2]);
    log_manager.show_deal_history(count);
  }
}

void BookStore::CommandManager::command_sellout(const ArglistType &argv) {
  // "buy [ISBN] [Quantity]"
  expect(argv.size()).toBe(3);
  expect(argv[1]).toBeConsistedOf(ascii_alphabet);
  expect(argv[2]).toBeConsistedOf(digit_alphabet);
  QuantityType quantity = std::stoll(argv[2]);
  LogType log = book_manager.sellout(ISBNType(argv[1]), quantity);

  log_manager.add_log(log, 1);
}

void BookStore::CommandManager::command_select_book(const ArglistType &argv) {
  // "select [ISBN]"
  expect(argv.size()).toBe(2);
  expect(argv[1]).toBeConsistedOf(ascii_alphabet);
  book_manager.select_book(ISBNType(argv[1]));
}

void BookStore::CommandManager::command_modify_book(const ArglistType &argv) {
  // "modify (-ISBN=[ISBN] | -name="[BookName]" | -author="[Author]" | -keyword="[Keyword]" | -price=[Price])+"
  expect(argv.size()).toBeOneOf(2, 3, 4, 5, 6);
  std::string ISBN, bookname, author, keyword_list;
  PriceType price = 0;
  bool is_modified[5] = {false, false, false, false, false};
  for(size_t i = 1; i < argv.size(); ++i) {
    std::smatch match;
    if(std::regex_search(argv[i], match, ISBN_aug_regex)) {
      if(is_modified[0])
        throw StarryPurple::ValidatorException();
      ISBN = match[1];
      expect(ISBN).toBeConsistedOf(ascii_alphabet);
      is_modified[0] = true;
    } else if(std::regex_search(argv[i], match, bookname_aug_regex)) {
      if(is_modified[1])
        throw StarryPurple::ValidatorException();
      bookname = match[1];
      expect(bookname).toBeConsistedOf(ascii_no_double_quotaton_alphabet);
      is_modified[1] = true;
    } else if(std::regex_search(argv[i], match, author_aug_regex)) {
      if(is_modified[2])
        throw StarryPurple::ValidatorException();
      author = match[1];
      expect(author).toBeConsistedOf(ascii_no_double_quotaton_alphabet);
      is_modified[2] = true;
    } else if(std::regex_search(argv[i], match, keyword_aug_regex)) {
      if(is_modified[3])
        throw StarryPurple::ValidatorException();
      keyword_list = match[1];
      expect(keyword_list).toBeConsistedOf(ascii_no_double_quotaton_alphabet);
      is_modified[3] = true;
    } else if(std::regex_search(argv[i], match, price_aug_regex)) {
      if(is_modified[4])
        throw StarryPurple::ValidatorException();
      std::string price_str = match[1];
      expect(price_str).toBeConsistedOf(digit_with_dot_alphabet);
      price = std::stod(price_str);
      is_modified[4] = true;
    } else throw StarryPurple::ValidatorException();
  }
  book_manager.modify_book(
    ISBNType(ISBN), BookInfoType(bookname), BookInfoType(author),
    BookInfoType(keyword_list), price, is_modified);
}

void BookStore::CommandManager::command_restock(const ArglistType &argv) {
  // “import [Quantity] [TotalCost]”
  expect(argv.size()).toBe(3);
  expect(argv[1]).toBeConsistedOf(digit_alphabet);
  expect(argv[2]).toBeConsistedOf(digit_with_dot_alphabet);
  QuantityType quantity = 0;
  PriceType price = 0.0;
  try {
    quantity = std::stoll(argv[1]);
    price = std::stod(argv[2]);
  } catch(std::out_of_range &) {
    throw std::runtime_error("Dude...");
    return;
  }
  LogType log = book_manager.restock(quantity, price);
  log_manager.add_log(log, 1);
}

void BookStore::CommandManager::command_show_log(const ArglistType &argv) {
  // “log”
  expect(argv.size()).toBe(1);
  log_manager.report_history();
}

void BookStore::CommandManager::command_show_report(const ArglistType &argv) {
  // “report finance”, "report employee"
  expect(argv.size()).toBe(2);
  if(argv[1] == "finance")
    log_manager.report_finance();
  if(argv[1] == "employee")
    log_manager.report_employee();
  else throw StarryPurple::ValidatorException();
}

void BookStore::CommandManager::open(const std::string &prefix) {
  if(is_running) close();
  user_manager.open(prefix + "_user");
  book_manager.open(prefix + "_book");
  log_manager.open(prefix + "_log");
  is_running = true;

  book_manager.user_stack_ptr = log_manager.user_stack_ptr = &user_manager.user_stack;

}

void BookStore::CommandManager::close() {
  if(!is_running) return;
  user_manager.close();
  book_manager.close();
  log_manager.close();
  is_running = false;
}

BookStore::CommandManager::~CommandManager() {
  if(is_running) close();
}


BookStore::CommandManager::ArglistType
BookStore::CommandManager::command_splitter(const std::string &command) {
  std::vector<std::string> argv;
  std::string arg;
  for(const char &ch: command) {
    if(ch == ' ' || ch == '\n' || ch == '\r' || ch == EOF) { // is EOF judge trully needed?
      if(!arg.empty()) {
        argv.push_back(arg);
        arg.clear();
      }
    } else arg += ch;
  }
  if(!arg.empty()) argv.push_back(arg);
  return argv;
}

void BookStore::CommandManager::command_list_reader(const std::string &prefix, const std::string &directory) {
  LogType::log_count = 0;
  open(directory + prefix);
  std::string command;
  while(std::getline(std::cin, command)) {
    ArglistType argv = command_splitter(command);
    if(argv.empty()) continue;
    try {
      if(argv[0] == "quit" || argv[0] == "exit") {
        // “quit”, "exit"
        expect(argv.size()).toBe(1);
        break;
      }
      if(argv[0] == "su")
        command_login(argv);
      else if(argv[0] == "logout")
        command_logout(argv);
      else if(argv[0] == "register")
        command_user_register(argv);
      else if(argv[0] == "passwd")
        command_change_password(argv);
      else if(argv[0] == "useradd")
        command_user_add(argv);
      else if(argv[0] == "delete")
        command_user_unregister(argv);
      else if(argv[0] == "show") {
        if(argv[1] == "finance")
          command_show_finance(argv);
        else command_list_book(argv);
      } else if(argv[0] == "buy")
        command_sellout(argv);
      else if(argv[0] == "select")
        command_select_book(argv);
      else if(argv[0] == "modify")
        command_modify_book(argv);
      else if(argv[0] == "import")
        command_restock(argv);
      else if(argv[0] == "log")
        command_show_log(argv);
      else if(argv[0] == "report")
        command_show_report(argv);
      else throw StarryPurple::ValidatorException();
    } catch(StarryPurple::ValidatorException &) {
      std::cout << "Invalid\n";
    }/* catch(std::out_of_range &) {
      std::cout << "Debug fail";
    }*/
  }
  close();
}
