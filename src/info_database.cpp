#include "info_database.h"

BookStore::UserStack::~UserStack() {
  if(is_open) close();
}

void BookStore::UserStack::open(const std::string &prefix) {
  if(is_open) close();
  u_stack.open(prefix + ".bsdat");
  is_open = true;
}

void BookStore::UserStack::close() {
  if(!is_open) return;
  u_stack.close();
  is_open = false;
}

BookStore::LoggedUsrType::LoggedUsrType(const UserType &usr)
  : user(usr) {}


void BookStore::UserStack::user_login(const UserType &user) {
  u_stack.push(LoggedUsrType(user));
  logged_set.insert(user.ID);
}

void BookStore::UserStack::user_logout() {
  if(u_stack.empty())
    throw StarryPurple::ValidatorException();
  logged_set.erase(u_stack.top().user.ID);
  u_stack.pop();
}

void BookStore::UserStack::user_select_book(const BookType &book) {
  if(u_stack.empty())
    throw StarryPurple::ValidatorException();
  u_stack.top().has_selected_book = true;
  u_stack.top().book_selected = book;
}

bool BookStore::UserStack::empty() const {
  return u_stack.empty();
}

BookStore::LoggedUsrType
&BookStore::UserStack::active_usr() {
  if(u_stack.empty())
    throw StarryPurple::ValidatorException(); // needed?
  return u_stack.top();
}

const BookStore::UserPrivilege
&BookStore::UserStack::active_privilege() {
  if(u_stack.empty())
    return UserPrivilege(0);
  return u_stack.top().user.privilege;
}





BookStore::UserDatabase::~UserDatabase() {
  if(is_open) close();
}

void BookStore::UserDatabase::open(const std::string &prefix) {
  if(is_open) close();
  bool is_existing_system = user_data.open(prefix + "_user.bsdat");
  id_map.open(prefix + "_user_id_map.bsdat");
  is_open = true;

  user_data.read_info(user_number);
  if(!is_existing_system)
    user_register(
      UserType(
        UserInfoType("root"), PasswordType("sjtu"),
        7, UserInfoType("Keeper")));
}

void BookStore::UserDatabase::close() {
  if(!is_open) return;
  user_data.write_info(user_number);
  id_map.close();
  is_open = false;
}

void BookStore::UserDatabase::user_register(const UserType &user) {
  UserPtr user_ptr = user_data.allocate(user);
  id_map.insert(user.ID, user_ptr);
  ++user_number;
}

void BookStore::UserDatabase::user_unregister(const UserInfoType &userID) {
  // todo: what if we unregister root?
  std::vector<UserPtr> user_ptr_list = id_map[userID];
  assert(user_ptr_list.size() == 1);
  UserPtr user_ptr = user_ptr_list[0];
  id_map.erase(userID, user_ptr);
  user_data.free(user_ptr);
  --user_number;
}






BookStore::BookDatabase::~BookDatabase() {
  if(is_open) close();
}

void BookStore::BookDatabase::open(const std::string &prefix) {
  if(is_open) close();
  book_data.open(prefix + "_book.bsdat");
  ID_map.open(prefix + "_book_id_map.bsdat");
  ISBN_map.open(prefix + "_book_isbn_map.bsdat");
  bookname_map.open(prefix + "_book_bookname_map.bsdat");
  author_map.open(prefix + "_book_author_map.bsdat");
  keyword_map.open(prefix + "_book_keyword_map.bsdat");
  is_open = true;

  book_data.read_info(book_number);
}

void BookStore::BookDatabase::close() {
  if(!is_open) return;
  book_data.write_info(book_number);
  book_data.close();
  ID_map.close();
  ISBN_map.close();
  bookname_map.close();
  author_map.close();
  keyword_map.close();
  is_open = false;
}

std::vector<BookStore::BookInfoType> BookStore::BookDatabase::keyword_splitter(const BookInfoType &keyword_list) {
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
  BookPtr book_ptr = book_data.allocate(book);
  ID_map.insert(book_number++, book_ptr);
  ISBN_map.insert(book.isbn, book_ptr);
  bookname_map.insert(book.bookname, book_ptr);
  author_map.insert(book.author, book_ptr);
  for(const auto &keyword: keyword_splitter(book.keyword_list))
    keyword_map.insert(keyword, book_ptr);
}

void BookStore::BookDatabase::book_modify_info(
  const BookPtr &book_ptr, const BookType &new_book, bool is_modified[5]) {
  BookType book; book_data.read(book, book_ptr);
  if(is_modified[0]) {
    ISBN_map.erase(book.isbn, book_ptr);
    book.isbn = new_book.isbn;
    ISBN_map.insert(book.isbn, book_ptr);
  }
  if(is_modified[1]) {
    bookname_map.erase(book.bookname, book_ptr);
    book.bookname = new_book.bookname;
    bookname_map.insert(book.bookname, book_ptr);
  }
  if(is_modified[2]) {
    author_map.erase(book.author, book_ptr);
    book.author = new_book.author;
    author_map.insert(book.author, book_ptr);
  }
  if(is_modified[3]) {
    for(const auto &keyword: keyword_splitter(book.keyword_list))
      keyword_map.erase(keyword, book_ptr);
    book.keyword_list = new_book.keyword_list;
    for(const auto &keyword: keyword_splitter(book.keyword_list))
      keyword_map.insert(keyword, book_ptr);
  }
  if(is_modified[4]) {
    // no price map now
    book.price = new_book.price;
  }
  book_data.write(book, book_ptr);
}






BookStore::LogDatabase::~LogDatabase() {
  if(is_open) close();
}

void BookStore::LogDatabase::open(const std::string &prefix) {
  if(is_open) close();
  bool is_exist = log_data.open(prefix + "_log.bsdat");
  all_log_id_map.open(prefix + "_log_id_map.bsdat");
  finance_log_id_map.open(prefix + "_log_finance_id_map.bsdat");
  employee_work_log_id_map.open(prefix + "_log_employee_work_map.bsdat");
  is_open = true;

  if(is_exist)
    log_data.read_info(info);
  else {
    info.employee_work_log_count = 0;
    info.finance_log_count = 0;
    info.all_log_count = 0;
    info.total_income = 0;
  }
}

void BookStore::LogDatabase::close() {
  if(!is_open) return;
  log_data.write_info(info);

  log_data.close();
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
  LogType log(income, expenditure, description);
  LogPtr log_ptr = log_data.allocate(log);
  all_log_id_map.insert(++info.all_log_count, log_ptr);
  if(log_level | 1)
    finance_log_id_map.insert(++info.finance_log_count, log_ptr);
  if(log_level | 2)
    employee_work_log_id_map.insert(++info.employee_work_log_count, log_ptr);
}

