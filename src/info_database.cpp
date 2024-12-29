#include "info_database.h"

#include <set>

BookStore::UserStack::~UserStack() {
  if(is_open) close();
}

void BookStore::UserStack::open(const std::string &prefix) {
  if(is_open) close();
  // u_stack.open(prefix + ".bsdat");
  is_open = true;
}

void BookStore::UserStack::close() {
  if(!is_open) return;
  // u_stack.close();
  // The data in std::vector will be automatically released and discarded.
  is_open = false;
}

void BookStore::UserStack::clear() {
  // u_stack.clear();
  // The data in std::vector will be automatically released and discarded.
  // but for the function of "clear", let's do some clearing.
  u_stack.empty();
  logged_set.clear();
}

BookStore::LoggedUserType::LoggedUserType(const UserType &user)
  : user_id(user.ID), privilege(user.privilege) {}


void BookStore::UserStack::user_login(const UserType &user) {
  u_stack.push_back(LoggedUserType(user));
  logged_set.insert(user.ID);
}

void BookStore::UserStack::user_logout() {
  if(u_stack.empty())
    throw StarryPurple::ValidatorException();
  logged_set.erase(u_stack.back().user_id);
  u_stack.pop_back();
}

void BookStore::UserStack::user_select_book(const ISBNType &ISBN) {
  if(u_stack.empty())
    throw StarryPurple::ValidatorException();
  u_stack.back().has_selected_book = true;
  u_stack.back().ISBN_selected = ISBN;
}

bool BookStore::UserStack::empty() const {
  return u_stack.empty();
}

BookStore::LoggedUserType
&BookStore::UserStack::active_user() {
  if(u_stack.empty())
    throw StarryPurple::ValidatorException(); // needed?
  return u_stack.back();
}

const BookStore::UserPrivilege
BookStore::UserStack::active_privilege() {
  if(u_stack.empty())
    return UserPrivilege(0);
  return u_stack.back().privilege;
}

void BookStore::UserStack::update_ISBN(const ISBNType &old_ISBN, const ISBNType &modified_ISBN) {
  for(auto &logged_user: u_stack)
    if(logged_user.has_selected_book && logged_user.ISBN_selected == old_ISBN)
      logged_user.ISBN_selected = modified_ISBN;
}




BookStore::UserDatabase::~UserDatabase() {
  if(is_open) close();
}

void BookStore::UserDatabase::open(const std::string &prefix) {
  if(is_open) close();
  bool is_exist = user_id_map.open(prefix + "_user_id_map.bsdat");
  is_open = true;

  if(!is_exist)
    user_register(
      UserType(
        UserInfoType("root"), PasswordType("sjtu"),
        7, UserInfoType("Keeper")));
}

void BookStore::UserDatabase::close() {
  if(!is_open) return;
  user_id_map.close();
  is_open = false;
}

void BookStore::UserDatabase::user_register(const UserType &user) {
  user_id_map.insert(user.ID, user);
}

void BookStore::UserDatabase::user_unregister(const UserInfoType &userID) {
  // todo: what if we unregister root?
  std::vector<UserType> user_list = user_id_map[userID];
  assert(user_list.size() == 1);
  UserType user = user_list[0];
  user_id_map.erase(userID, user);
}






BookStore::BookDatabase::~BookDatabase() {
  if(is_open) close();
}

void BookStore::BookDatabase::open(const std::string &prefix) {
  if(is_open) close();
  book_map.open(prefix + "_book_id_map.bsdat");
  ISBN_map.open(prefix + "_book_isbn_map.bsdat");
  bookname_map.open(prefix + "_book_bookname_map.bsdat");
  author_map.open(prefix + "_book_author_map.bsdat");
  keyword_map.open(prefix + "_book_keyword_map.bsdat");
  is_open = true;

}

void BookStore::BookDatabase::close() {
  if(!is_open) return;
  book_map.close();
  ISBN_map.close();
  bookname_map.close();
  author_map.close();
  keyword_map.close();
  is_open = false;
}

std::vector<BookStore::BookInfoType> BookStore::BookDatabase::keyword_splitter(
  const BookInfoType &keyword_list) {
  std::vector<BookInfoType> keyword_vector;
  std::string keyword;
  for(int i = 0; i < keyword_list.length(); ++i) {
    if(keyword_list[i] == '|') {
      if(!keyword.empty()) {
        keyword_vector.push_back(BookInfoType(keyword));
        keyword.clear();
      }
    } else keyword += keyword_list[i];
  }
  if(!keyword.empty())
    keyword_vector.push_back(BookInfoType(keyword));
  return keyword_vector;
}


void BookStore::BookDatabase::book_register(const BookType &book) {
  expect(ISBN_map[book.isbn].size()).toBe(0);
  book_map.insert(0, book);
  ISBN_map.insert(book.isbn, book);
  bookname_map.insert(book.bookname, book);
  author_map.insert(book.author, book);
  for(const auto &keyword: keyword_splitter(book.keyword_list))
    keyword_map.insert(keyword, book);
}

void BookStore::BookDatabase::book_modify_info(
  const BookType &old_book, BookType &modified_book, bool is_modified[6]) {
  if(!is_modified[0]) modified_book.isbn = old_book.isbn;
  else expect(ISBN_map[modified_book.isbn].size()).toBe(0);
  if(!is_modified[1]) modified_book.bookname = old_book.bookname;
  if(!is_modified[2]) modified_book.author = old_book.author;
  if(!is_modified[3])
    modified_book.keyword_list = old_book.keyword_list;
  else {
    std::vector<UserInfoType> keyword_vector = keyword_splitter(modified_book.keyword_list);
    std::set<UserInfoType> keyword_set;
    for(const auto &keyword: keyword_vector)
      keyword_set.insert(keyword);
    expect(keyword_set.size()).toBe(keyword_vector.size());
  }
  if(!is_modified[4]) modified_book.price = old_book.price;
  if(!is_modified[5]) modified_book.storage = old_book.storage;

  book_map.erase(0, old_book);
  ISBN_map.erase(old_book.isbn, old_book);
  bookname_map.erase(old_book.bookname, old_book);
  author_map.erase(old_book.author, old_book);
  for(const auto &keyword: keyword_splitter(old_book.keyword_list))
    keyword_map.erase(keyword, old_book);

  book_map.insert(0, modified_book);
  ISBN_map.insert(modified_book.isbn, modified_book);
  bookname_map.insert(modified_book.bookname, modified_book);
  author_map.insert(modified_book.author, modified_book);
  for(const auto &keyword: keyword_splitter(modified_book.keyword_list))
    keyword_map.insert(keyword, modified_book);
}


void BookStore::BookDatabase::book_change_storage(
  const BookType &book, const QuantityType &quantity) {
  BookType modified_book;
  modified_book.storage = book.storage + quantity;
  expect(modified_book.storage).greaterEqual(0);
  bool is_modified[6] = {false, false, false, false, false, true};
  book_modify_info(book, modified_book, is_modified);
}




BookStore::LogDatabase::~LogDatabase() {
  if(is_open) close();
}

void BookStore::LogDatabase::open(const std::string &prefix) {
  if(is_open) close();
  bool is_exist = log_info.open(prefix + "_log.bsdat");
  all_log_id_map.open(prefix + "_log_id_map.bsdat");
  finance_log_id_map.open(prefix + "_log_finance_id_map.bsdat");
  employee_work_log_id_map.open(prefix + "_log_employee_work_map.bsdat");
  is_open = true;

  if(is_exist)
    log_info.read_info(info);
  else {
    info.employee_work_log_count = 0;
    info.finance_log_count = 0;
    info.all_log_count = 0;
    info.total_income = 0;
  }
}

void BookStore::LogDatabase::close() {
  if(!is_open) return;
  log_info.write_info(info);

  log_info.close();
  all_log_id_map.close();
  finance_log_id_map.close();
  employee_work_log_id_map.close();
  is_open = false;
}

void BookStore::LogDatabase::add_log(
  const PriceType &income, const PriceType &expenditure,
  const LogDescriptionType &description, int log_level) {
  info.total_income += income;
  info.total_expenditure += expenditure;
  LogType log(info.total_income, info.total_expenditure, description);
  all_log_id_map.insert(++info.all_log_count, log);
  if(log_level & 1)
    finance_log_id_map.insert(++info.finance_log_count, log);
  if(log_level & 2)
    employee_work_log_id_map.insert(++info.employee_work_log_count, log);
}

