#include "info_manager.h"

#include <iomanip>

BookStore::UserManager::~UserManager() {
  if(is_running) close();
}

void BookStore::UserManager::open(const filenameType &prefix) {
  if(is_running) close();
  user_stack.open(prefix + "_stack");
  user_database.open(prefix + "_database");
  is_running = true;
}

void BookStore::UserManager::close() {
  if(!is_running) return;
  user_stack.close();
  user_database.close();
  is_running = false;
}

void BookStore::UserManager::login(const UserInfoType &userID, const PasswordType &password) {
  std::vector<UserPtr> user_ptr_vector = user_database.id_map[userID];
  expect(user_ptr_vector.size()).toBe(1);
  UserPtr user_ptr = user_ptr_vector[0];
  UserType user;
  user_database.user_data.read(user, user_ptr);
  expect(user.passwd).toBe(password);
  user_stack.user_login(user);
}


void BookStore::UserManager::login(const UserInfoType &userID) {
  std::vector<UserPtr> user_ptr_vector = user_database.id_map[userID];
  expect(user_ptr_vector.size()).toBe(1);
  UserPtr user_ptr = user_ptr_vector[0];
  UserType user;
  user_database.user_data.read(user, user_ptr);
  expect(user_stack.active_privilege()).Not().lesserEqual(user.privilege);
  user_stack.user_login(user);
}

void BookStore::UserManager::user_register(const UserType &user) {
  std::vector<UserPtr> user_ptr_vector = user_database.id_map[user.ID];
  expect(user_ptr_vector.size()).toBe(0);
  user_database.user_register(user);
}

void BookStore::UserManager::user_add(const UserType &user) {
  expect(user_stack.active_privilege()).Not().lesserEqual(UserPrivilege(3));
  expect(user_stack.active_privilege()).Not().lesserEqual(user.privilege);
  std::vector<UserPtr> user_ptr_vector = user_database.id_map[user.ID];
  expect(user_ptr_vector.size()).toBe(1);
  user_database.user_register(user);
}



void BookStore::UserManager::change_password(
    const UserInfoType &userID,
    const PasswordType &cur_pwd, const PasswordType &new_pwd) {
  expect(user_stack.active_privilege()).Not().lesserEqual(UserPrivilege(1));
  std::vector<UserPtr> user_ptr_vector = user_database.id_map[userID];
  expect(user_ptr_vector.size()).toBe(1);
  UserPtr user_ptr = user_ptr_vector[0];
  UserType user;
  user_database.user_data.read(user, user_ptr);
  expect(user.passwd).toBe(cur_pwd);
  user.passwd = new_pwd;
  user_database.user_data.write(user, user_ptr);
}

void BookStore::UserManager::change_password(
  const UserInfoType &userID,
  const PasswordType &new_pwd) {
  expect(user_stack.active_privilege()).greaterEqual(UserPrivilege(7));
  std::vector<UserPtr> user_ptr_vector = user_database.id_map[userID];
  expect(user_ptr_vector.size()).toBe(1);
  UserPtr user_ptr = user_ptr_vector[0];
  UserType user;
  user_database.user_data.read(user, user_ptr);
  user.passwd = new_pwd;
  user_database.user_data.write(user, user_ptr);
}

void BookStore::UserManager::logout() {
  expect(user_stack.active_privilege()).Not().lesserEqual(UserPrivilege(1));
  expect(user_stack.empty()).toBe(false);
  user_stack.user_logout();
}

void BookStore::UserManager::user_unregister(const UserInfoType &userID) {
  expect(user_stack.active_privilege()).Not().lesserEqual(UserPrivilege(7));
  std::vector<UserPtr> user_ptr_vector = user_database.id_map[userID];
  expect(user_ptr_vector.size()).toBe(1);
  UserPtr user_ptr = user_ptr_vector[0];
  UserType user;
  user_database.user_data.read(user, user_ptr);
  expect(user_stack.logged_set.count(userID)).toBe(0);
  user_database.user_unregister(userID);
}


BookStore::BookManager::~BookManager() {
  if(is_running) close();
}

void BookStore::BookManager::open(const filenameType &prefix) {
  if(is_running) close();
  book_database.open(prefix + "_database");
  is_running = true;
}

void BookStore::BookManager::close() {
  if(!is_running) return;
  book_database.close();
  is_running = false;
}

void BookStore::BookManager::select_book(const ISBNType &ISBN, LoggedUsrType &active_user) {
  expect(user_stack_ptr->active_privilege()).Not().lesserEqual(UserPrivilege(3));
  auto book_ptr_vector = book_database.ISBN_map[ISBN];
  expect(book_ptr_vector.size()).toBe(1);
  BookType book;
  book_database.book_data.read(book, book_ptr_vector[0]);
  active_user.book_selected = book;
  active_user.has_selected_book = true;
}

void BookStore::BookManager::list_all() {
  expect(user_stack_ptr->active_privilege()).Not().lesserEqual(UserPrivilege(1));
  for(size_t i = 1; i <= book_database.book_number; ++i) {

  }
}

void BookStore::BookManager::list_ISBN(const ISBNType &ISBN) {

}

void BookStore::BookManager::list_bookname(const BookInfoType &bookname) {

}

void BookStore::BookManager::list_author(const BookInfoType &author) {

}

void BookStore::BookManager::list_keyword(const BookInfoType &keyword) {

}

void BookStore::BookManager::restock(const QuantityType &quantity, const PriceType &total_cost) {

}

void BookStore::BookManager::sellout(const ISBNType &ISBN, const QuantityType &quantity) {

}

void BookStore::BookManager::modify_book(
  const ISBNType &ISBN, const BookInfoType &bookname,const BookInfoType &author,
  const BookInfoType &keyword_list, const PriceType &price, bool is_modified[5]) {

}



BookStore::LogManager::~LogManager() {
  if(is_running) close();
}

void BookStore::LogManager::open(const filenameType &prefix) {
  if(is_running) close();
  log_database.open(prefix + "_database");
  is_running = true;
}

void BookStore::LogManager::close() {
  if(!is_running) return;
  log_database.close();
  is_running = false;
}

void BookStore::LogManager::show_deal_history(const LogCountType &count) {
  expect(count).lesserEqual(log_database.info.finance_log_count);
  if(count == 0) {
    std::cout << '\n';
    return;
  }
  if(count == log_database.info.finance_log_count) {
    show_deal_history();
    return;
  }
  LogType history_log;
  LogPtr history_log_ptr = log_database.all_log_id_map[log_database.info.finance_log_count - count][0];
  log_database.log_data.read(history_log, history_log_ptr);
  std::cout << "+ " << std::fixed << std::setprecision(2) <<
    log_database.info.total_income - history_log.total_income
  << " - " << log_database.info.total_expenditure - history_log.total_expenditure << '\n';
}

void BookStore::LogManager::show_deal_history() {
  if(log_database.info.finance_log_count == 0) {

  }
  std::cout << "+ " << std::fixed << std::setprecision(2) <<
    log_database.info.total_income << " - " << log_database.info.total_expenditure << '\n';
}

void BookStore::LogManager::report_finance() {
  std::cout << "Now reporting finance history.\n";
  LogType log;
  LogPtr log_ptr;
  PriceType history_income = 0, history_expenditure = 0;
  for(size_t i = 1; i <= log_database.info.finance_log_count; ++i) {
    log_ptr = log_database.finance_log_id_map[i][0];
    log_database.log_data.read(log, log_ptr);

    if(log.total_income - history_income != 0)
      std::cout << "Earned: " << std::fixed << std::setprecision(2) <<
        log.total_income - history_income << '\n';
    else if(log.total_expenditure - history_expenditure != 0)
      std::cout << "Used: " << std::fixed << std::setprecision(2) <<
        log.total_expenditure - history_expenditure << '\n';

    history_income = log.total_income;
    history_expenditure = log.total_expenditure;
  }
  std::cout << '\n' << "Total history income: " << std::fixed << std::setprecision(2) <<
    history_income << '\n' << "Total history expenditure: " <<
      history_expenditure << '\n';
}

void BookStore::LogManager::report_employee() {
  std::cout << "Now reporting employee working history.\n";
  LogType log;
  LogPtr log_ptr;
  for(size_t i = 1; i <= log_database.info.employee_work_log_count; ++i) {
    log_ptr = log_database.employee_work_log_id_map[i][0];
    log_database.log_data.read(log, log_ptr);
    std::cout << log.log_description.to_str() << '\n';
  }
  std::cout << '\n';
}

void BookStore::LogManager::report_history() {
  std::cout << "Now reporting system history.\n";
  LogType log;
  LogPtr log_ptr;
  for(size_t i = 1; i <= log_database.info.all_log_count; ++i) {
    log_ptr = log_database.all_log_id_map[i][0];
    log_database.log_data.read(log, log_ptr);
    std::cout << log.log_description.to_str() << '\n';
  }
  std::cout << '\n';
}
