/** filestream.h
 * Author: StarryPurple
 * Date: Since 2024.12.16
 *
 * This file contains a self-made file stream class and its pointer.
 * Class Requirement: the filename, the data structure type (StorageType) in declaration,
 *     and the no-parameter default constructor of StorageType.
 *
 * You can also assign:
 *     The number of elements in this file. Initially cElementCount.
 *     The type of extra information reserved by user (InfoType). Initially size_t.
 *
 * I also placed a cMaxFileSize here to restrict the size of a Fstream file.
 * Maybe removed later.
 *
 * structure of Fstream-related files:
 * 1. Reserved information space: InfoType
 *        Store some extra information that may be needed. Exactly one InfoType object.
 * 1. Last recently used sign: size_t
 *        the last visited storage location.
 * 2. A bitmap for storage usage: bool [cCapacity]
 *        A boolean sign is true if and only if correlated storage has been occupied.
 * 3. The storage body: StorageType [cCapacity]
 *        Where these data are stored.
 *
 * The whole size of the file is determined since its creation by StorageType and cElementCount.
 * as we'll initialize it with empty StorageTypes.
 */
#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include "bookstore_exceptions.h"

#include <fstream>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

namespace StarryPurple {

using offsetType = int;
using filenameType = std::string;
constexpr size_t cMaxFileSize = 1 << 22; // 32 MB
constexpr size_t cCapacity = 1 << 14; // 16384, > 10000

template<class StorageType, class InfoType, size_t capacity>
class Fstream;

template<size_t capacity>
class Fpointer {

public:
  // template<class StorageType, class InfoType> friend Fstream<StorageType, InfoType, elementCount>;

  // the default constructor set offset_ = elementCount
  // to ensure that the initial Fpointer is invalid, like nullptr.
  Fpointer();
  explicit Fpointer(nullptr_t);
  explicit Fpointer(offsetType offset);
  ~Fpointer() = default;

  void setnull();
  bool isnull() const;
  bool operator==(const Fpointer &other) const;
  bool operator!=(const Fpointer &other) const;

  offsetType offset_ = capacity;
};

template<class StorageType, class InfoType = size_t, size_t capacity = cCapacity>
class Fstream {
  // allow for sizeof(StorageType) at approximately cMaxFileSize / cElementCount = 1 << 7 = 128
  // smaller than 4KB ~ 4096
  // static_assert(
  //   sizeof(InfoType) + sizeof(size_t) + (sizeof(StorageType) + sizeof(bool)) * cElementCount <= cMaxFileSize);

public:

  using fpointer = Fpointer<capacity>;

  Fstream() = default;
  ~Fstream();

  // open a file.
  // return whether the file exists before.
  bool open(const std::string &filename);
  // close the currently opened file.
  void close();

  // allocate a storage block and initial it with given object.
  fpointer allocate();
  // allocate a storage block and initial it with an empty one.
  fpointer allocate(const StorageType &data);
  // free the storage block.
  void free(const fpointer &ptr);

  // write an object into the assigned location.
  void write(const StorageType &data, const fpointer &ptr);
  // read an object from the assigned location.
  void read(StorageType &data, const fpointer &ptr);

  // write the info.
  void write_info(const InfoType &info);
  // read the info.
  void read_info(InfoType &info);

private:
  const size_t cStorageSize = sizeof(StorageType);
  const size_t cExtraInfoSize = sizeof(InfoType);
  const size_t cInfoSize = cExtraInfoSize + sizeof(size_t) + sizeof(bool) * capacity;
  const size_t cFileSize = cInfoSize + cStorageSize * capacity;
  InfoType extra_info_;
  offsetType lru_loc_ = 0;
  bool bitmap_[capacity]{};
  std::fstream file_{};
  std::string filename_;

};

} // namespace StarryPurple

#include "filestream.tpp"

#endif // FILE_STREAM_H