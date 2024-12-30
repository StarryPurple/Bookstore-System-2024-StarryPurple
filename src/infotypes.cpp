#include "infotypes.h"

#include <iomanip>

BookStore::UserPrivilege::UserPrivilege(int privilege)
  : pri_1(privilege & 1), pri_2(privilege & 2), pri_3(privilege & 4) {}

BookStore::UserPrivilege::UserPrivilege(const UserPrivilege &other)
  : pri_1(other.pri_1), pri_2(other.pri_2), pri_3(other.pri_3) {}

bool BookStore::UserPrivilege::operator==(const UserPrivilege &other) const {
  return (pri_1 == other.pri_1) && (pri_2 == other.pri_2) && (pri_3 == other.pri_3);
}

bool BookStore::UserPrivilege::operator!=(const UserPrivilege &other) const {
  return !(*this == other);
}

bool BookStore::UserPrivilege::operator<=(const UserPrivilege &other) const {
  return !((!other.pri_1 && pri_1) || (!other.pri_2 && pri_2) || (!other.pri_3 && pri_3));
}

bool BookStore::UserPrivilege::operator>=(const UserPrivilege &other) const {
  return !((!pri_1 && other.pri_1) || (!pri_2 && other.pri_2) || (!pri_3 && other.pri_3));
}

BookStore::BookType::BookType(const ISBNType &ISBN) : isbn(ISBN) {}

bool BookStore::BookType::operator==(const BookType &other) const {
  return isbn == other.isbn;
}

bool BookStore::BookType::operator!=(const BookType &other) const {
  return isbn != other.isbn;
}

bool BookStore::BookType::operator<(const BookType &other) const {
  return isbn < other.isbn;
}

bool BookStore::BookType::operator>(const BookType &other) const {
  return isbn > other.isbn;
}

bool BookStore::BookType::operator<=(const BookType &other) const {
  return isbn <= other.isbn;
}

bool BookStore::BookType::operator>=(const BookType &other) const {
  return isbn >= other.isbn;
}

void BookStore::BookType::print() const {
  std::cout << isbn.to_str() << '\t' << bookname.to_str() << '\t' << author.to_str() <<
    '\t' << keyword_list.to_str() << '\t' << std::fixed << std::setprecision(2) <<
      price << '\t' << storage << '\n';
}

std::string BookStore::BookType::book_identity_str() const {
  return '\"' + bookname.to_str() + "\" (ISBN: " + isbn.to_str() + ")";
}


BookStore::UserType::UserType(
  const UserInfoType &userID, const PasswordType &password,
  int user_privilege, const UserInfoType &name)
    : user_id(userID), passwd(password), privilege(user_privilege), username(name) {}

std::string BookStore::UserType::user_identity_str() const {
  std::string res = "\"";
  if(privilege.pri_3) res += "Keeper";
  else if(privilege.pri_2) res += "Worker";
  else if(privilege.pri_1) res += "Client";
  else res += "Tourist";
  res += "\" " + username.to_str() + " (id: " + user_id.to_str() + ")";
  return res;
}


bool BookStore::UserType::operator==(const UserType &other) const {
  return user_id == other.user_id;
}

bool BookStore::UserType::operator!=(const UserType &other) const {
  return user_id != other.user_id;
}

bool BookStore::UserType::operator<(const UserType &other) const {
  return user_id < other.user_id;
}

bool BookStore::UserType::operator>(const UserType &other) const {
  return user_id > other.user_id;
}

bool BookStore::UserType::operator<=(const UserType &other) const {
  return user_id <= other.user_id;
}

bool BookStore::UserType::operator>=(const UserType &other) const {
  return user_id >= other.user_id;
}

size_t BookStore::LogType::log_count = 0;

BookStore::LogType::LogType(
  const PriceType &history_income, const PriceType &history_expenditure,
  const LogDescriptionType &description)
    : id(0), total_income(history_income), total_expenditure(history_expenditure),
     log_description(description) {}

BookStore::LogType::LogType(
  const PriceType &history_income, const PriceType &history_expenditure,
  const LogDescriptionType &description, bool to_record)
    : id(++log_count), total_income(history_income), total_expenditure(history_expenditure),
     log_description(description) {}

bool BookStore::LogType::operator==(const LogType &other) const {
  return id == other.id;
}

bool BookStore::LogType::operator!=(const LogType &other) const {
  return id != other.id;
}

bool BookStore::LogType::operator<(const LogType &other) const {
  return id < other.id;
}

bool BookStore::LogType::operator>(const LogType &other) const {
  return id > other.id;
}

bool BookStore::LogType::operator<=(const LogType &other) const {
  return id <= other.id;
}

bool BookStore::LogType::operator>=(const LogType &other) const {
  return id >= other.id;
}

