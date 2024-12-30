#include "info_manager.h"

#include <iomanip>
#include <set>

BookStore::UserManager::~UserManager() {
  if(is_running) close();
}

void BookStore::UserManager::open(const std::string &prefix) {
  if(is_running) close();
  user_stack.open(prefix + "_stack");
  user_database.open(prefix + "_database");
  is_running = true;
}

void BookStore::UserManager::close() {
  if(!is_running) return;

  user_stack.clear();
  user_stack.close();
  user_database.close();
  is_running = false;
}

void BookStore::UserManager::login(const UserInfoType &userID, const PasswordType &password) {
  std::vector<UserType> user_vector = user_database.user_id_map[userID];
  expect(user_vector.size()).toBe(1);
  UserType user = user_vector[0];
  user_stack.user_login(user);
  expect(user.passwd).toBe(password);
}


void BookStore::UserManager::login(const UserInfoType &userID) {
  std::vector<UserType> user_vector = user_database.user_id_map[userID];
  expect(user_vector.size()).toBe(1);
  UserType user = user_vector[0];
  expect(user_stack.active_privilege()).greaterEqual(user.privilege);
  user_stack.user_login(user);
}

void BookStore::UserManager::user_register(const UserType &user) {
  std::vector<UserType> user_vector = user_database.user_id_map[user.ID];
  expect(user_vector.size()).toBe(0);
  user_database.user_register(user);
}

void BookStore::UserManager::user_add(const UserType &user) {
  expect(user_stack.active_privilege()).greaterEqual(UserPrivilege(3));
  expect(user_stack.active_privilege()).Not().lesserEqual(user.privilege);
  std::vector<UserType> user_vector = user_database.user_id_map[user.ID];
  expect(user_vector.size()).toBe(0);
  user_database.user_register(user);
}



void BookStore::UserManager::change_password(
    const UserInfoType &userID,
    const PasswordType &cur_pwd, const PasswordType &new_pwd) {
  expect(user_stack.active_privilege()).greaterEqual(UserPrivilege(1));
  std::vector<UserType> user_vector = user_database.user_id_map[userID];
  expect(user_vector.size()).toBe(1);
  UserType user = user_vector[0];
  expect(user.passwd).toBe(cur_pwd);
  user_database.user_id_map.erase(userID, user);
  user.passwd = new_pwd;
  user_database.user_id_map.insert(userID, user);
}

void BookStore::UserManager::change_password(
  const UserInfoType &userID,
  const PasswordType &new_pwd) {
  expect(user_stack.active_privilege()).greaterEqual(UserPrivilege(7));
  std::vector<UserType> user_vector = user_database.user_id_map[userID];
  expect(user_vector.size()).toBe(1);
  UserType user = user_vector[0];
  user_database.user_id_map.erase(userID, user);
  user.passwd = new_pwd;
  user_database.user_id_map.insert(userID, user);
}

void BookStore::UserManager::logout() {
  expect(user_stack.active_privilege()).greaterEqual(UserPrivilege(1));
  expect(user_stack.empty()).toBe(false);
  user_stack.user_logout();
}

void BookStore::UserManager::user_unregister(const UserInfoType &userID) {
  expect(user_stack.active_privilege()).greaterEqual(UserPrivilege(7));
  expect(user_stack.logged_set.count(userID)).toBe(0);
  user_database.user_unregister(userID);
}







BookStore::BookManager::~BookManager() {
  if(is_running) close();
}

void BookStore::BookManager::open(const std::string &prefix) {
  if(is_running) close();
  book_database.open(prefix + "_database");
  is_running = true;
}

void BookStore::BookManager::close() {
  if(!is_running) return;
  book_database.close();
  is_running = false;
}

void BookStore::BookManager::select_book(const ISBNType &ISBN) {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(3));
  std::vector<BookType> book_vector = book_database.ISBN_map[ISBN];
  if(book_vector.size() == 0) {
    BookType book;
    book.isbn = ISBN;
    book_database.book_register(book);
    user_stack_ptr->user_select_book(ISBN);
  } else if(book_vector.size() == 1) {
    user_stack_ptr->user_select_book(book_vector[0].isbn);
  } // else assert(false);
}

void BookStore::BookManager::list_all() {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(1));
  std::vector<BookType> book_vector = book_database.book_map[0];
  if(book_vector.empty())
    std::cout << '\n';
  else
    for(const auto &book: book_vector)
      book.print();
}


void BookStore::BookManager::list_ISBN(const ISBNType &ISBN) {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(1));
  expect(ISBN.empty()).toBe(false);
  std::vector<BookType> book_vector = book_database.ISBN_map[ISBN];
  if(book_vector.empty())
    std::cout << '\n';
  else
    for(const auto &book: book_vector)
      book.print();
}

void BookStore::BookManager::list_bookname(const BookInfoType &bookname) {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(1));
  expect(bookname.empty()).toBe(false);
  std::vector<BookType> book_vector = book_database.bookname_map[bookname];
  if(book_vector.empty())
    std::cout << '\n';
  else
    for(const auto &book: book_vector)
      book.print();
}

void BookStore::BookManager::list_author(const BookInfoType &author) {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(1));
  expect(author.empty()).toBe(false);
  std::vector<BookType> book_vector = book_database.author_map[author];
  if(book_vector.empty())
    std::cout << '\n';
  else
    for(const auto &book: book_vector)
      book.print();
}

void BookStore::BookManager::list_keyword(const BookInfoType &keyword) {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(1));
  expect(keyword.empty()).toBe(false);
  for(int i = 0; i < keyword.length(); ++i)
    expect(keyword[i]).Not().toBe('|');  // True?
  std::vector<BookType> book_vector = book_database.keyword_map[keyword];
  if(book_vector.empty())
    std::cout << '\n';
  else
    for(const auto &book: book_vector)
      book.print();
}

BookStore::LogType BookStore::BookManager::restock(
  const QuantityType &quantity, const PriceType &total_cost) {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(3));
  expect(user_stack_ptr->active_user().has_selected_book).toBe(true);
  expect(quantity).Not().lesserEqual(0);
  expect(total_cost).Not().lesserEqual(0.0);
  ISBNType ISBN = user_stack_ptr->active_user().ISBN_selected;
  std::vector<BookType> book_vector = book_database.ISBN_map[ISBN];
  expect(book_vector.size()).toBe(1);
  BookType book = book_vector[0];
  book_database.book_change_storage(book, quantity);
  return LogType(0, total_cost, LogDescriptionType("log command import"));
}

BookStore::LogType BookStore::BookManager::sellout(const ISBNType &ISBN, const QuantityType &quantity) {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(1));
  expect(quantity).Not().lesserEqual(0);
  std::vector<BookType> book_vector = book_database.ISBN_map[ISBN];
  expect(book_vector.size()).toBe(1);
  BookType book = book_vector[0];
  book_database.book_change_storage(book, -quantity); // remember this '-'
  std::cout << std::fixed << std::setprecision(2) << (book.price * quantity) << '\n';
  return LogType(book.price * quantity, 0, LogDescriptionType("log command buy"));
}

void BookStore::BookManager::modify_book(
  const ISBNType &ISBN, const BookInfoType &bookname,const BookInfoType &author,
  const BookInfoType &keyword_list, const PriceType &price, bool is_modified[5]) {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(3));
  expect(user_stack_ptr->active_user().has_selected_book).toBe(true);
  ISBNType old_ISBN = user_stack_ptr->active_user().ISBN_selected;
  std::vector<BookType> book_vector = book_database.ISBN_map[old_ISBN];
  // assert(book_vector.size() == 1); // Needed?
  expect(book_vector.size()).toBe(1);
  BookType old_book = book_vector[0];
  BookType modified_book;
  modified_book.isbn = ISBN; modified_book.bookname = bookname;
  modified_book.author = author;
  modified_book.keyword_list = keyword_list; modified_book.price = price;
  bool is_to_modify[6] =
    {is_modified[0], is_modified[1], is_modified[2], is_modified[3], is_modified[4], false};
  book_database.book_modify_info(old_book, modified_book, is_to_modify);
  if(is_modified[0]) {
    // modify all old_isbn in the user_stack to new_isbn.
    // modified_book here is a truthfully modified one, not with some uncertainties.
    user_stack_ptr->update_ISBN(old_ISBN, modified_book.isbn);
  }
}



BookStore::LogManager::~LogManager() {
  if(is_running) close();
}

void BookStore::LogManager::open(const std::string &prefix) {
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
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(7));
  expect(count).greaterEqual(0);
  expect(count).lesserEqual(log_database.info.finance_log_count);
  if(count == 0) {
    std::cout << '\n';
    return;
  }
  if(count == log_database.info.finance_log_count) {
    show_deal_history();
    return;
  }
  LogType history_log = log_database.all_log_id_map[log_database.info.finance_log_count - count][0];
  std::cout << "+ " << std::fixed << std::setprecision(2) <<
    log_database.info.total_income - history_log.total_income
  << " - " << log_database.info.total_expenditure - history_log.total_expenditure << '\n';
}

void BookStore::LogManager::show_deal_history() {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(7));
  std::cout << "+ " << std::fixed << std::setprecision(2) <<
    log_database.info.total_income << " - " << log_database.info.total_expenditure << '\n';
}

void BookStore::LogManager::report_finance() {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(7));
  std::cout << "Now reporting finance history.\n";
  LogType log;
  PriceType history_income = 0, history_expenditure = 0;
  for(size_t i = 1; i <= log_database.info.finance_log_count; ++i) {
    log = log_database.finance_log_id_map[i][0];

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
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(7));
  std::cout << "Now reporting employee working history.\n";
  LogType log;
  for(size_t i = 1; i <= log_database.info.employee_work_log_count; ++i) {
    log = log_database.employee_work_log_id_map[i][0];
    std::cout << log.log_description.to_str() << '\n';
  }
  std::cout << '\n';
}

void BookStore::LogManager::report_history() {
  expect(user_stack_ptr->active_privilege()).greaterEqual(UserPrivilege(7));
  std::cout << "Now reporting system history.\n";
  LogType log;
  for(size_t i = 1; i <= log_database.info.all_log_count; ++i) {
    log = log_database.all_log_id_map[i][0];
    std::cout << log.log_description.to_str() << '\n';
  }
  std::cout << '\n';
}

void BookStore::LogManager::add_log(
  const LogType &log, int log_level) {
  log_database.add_log(log.total_income, log.total_expenditure, log.log_description, log_level);
}
