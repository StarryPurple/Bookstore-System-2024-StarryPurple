#include "infotypes.h"

#include <iomanip>

BookStore::UserPrivilege::UserPrivilege(int privilege)
  : pri_1(privilege | 1), pri_2(privilege | 2), pri_3(privilege | 4) {}

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

BookStore::UserType::UserType(
  const UserInfoType &userID, const PasswordType &password,
  int user_privilege, const UserInfoType &name)
    : ID(userID), passwd(password), privilege(user_privilege), username(name) {}

BookStore::LogType::LogType(
  const PriceType &history_income, const PriceType &history_expenditure,
  const LogDescriptionType &description)
    :total_income(history_income), total_expenditure(history_expenditure),
     log_description(description) {}

