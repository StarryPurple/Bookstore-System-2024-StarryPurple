/** FileStream.h
 * Author: StarryPurple
 * Date: Since 2024.12.16
 *
 * This file contains a self-made file stream class and its pointer.
 * Class Requirement: the filename, the data structure type (StorageType) in declaration,
 *     and the no-parameter default constructor of StorageType.
 *
 * You can also assign the number of elements in this file. Initially cElementCount.
 *
 * I also placed a cMaxFileSize here to restrict the size of a Fstream file.
 * Maybe removed later.
 *
 * structure of Fstream-related files:
 * 1. Last Recently used sign: size_t
 *        the last visited storage location.
 * 2. A bitmap for storage usage: bool [cElementCount]
 *        A boolean sign is true if and only if correlated storage has been occupied.
 * 3. The storage body: StorageType [cElementCount]
 *        Where these data are stored.
 *
 * The whole size of the file is determined since its creation by StorageType and cElementCount.
 * as we'll initialize it with empty StorageTypes.
 */
#pragma once
#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include "BookstoreExceptions.h"

#include <fstream>
#include <cassert>
#include <cstring>

namespace StarryPurple_FileIO {

using offsetType = size_t;
using filenameType = std::string;
constexpr size_t cMaxFileSize = 1 << 21; // 16 MB
constexpr size_t cElementCount = 1 << 14; // 16384, > 10000

template<class StorageType, size_t elementCount> class Fpointer;
template<class StorageType, size_t elementCount> class Fstream;


// @offset: the ordinal of the storage block.
template<class StorageType, size_t elementCount>
class Fpointer {

public:
  friend Fstream<StorageType, elementCount>;

  Fpointer() = delete;
  Fpointer(offsetType offset);
  ~Fpointer() = default;

private:
  const offsetType offset_;
};

template<class StorageType, size_t elementCount = cElementCount>
class Fstream {
  // allow for sizeof(StorageType) at approximately cMaxFileSize / cElementCount = 1 << 7 = 128
  static_assert(sizeof(size_t) + (sizeof(StorageType) + sizeof(bool)) * cElementCount <= cMaxFileSize);

public:
  using fpointer = Fpointer<StorageType, elementCount>;

  Fstream() = default;
  ~Fstream();

  // open a file.
  // attention: filename should end with extension ".bsdat"
  void open(const filenameType &filename);
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

private:
  const size_t cStorageSize = sizeof(StorageType);
  const size_t cInfoSize = sizeof(size_t) + sizeof(bool) * elementCount;
  const size_t cFileSize = cInfoSize + cStorageSize * elementCount;
  size_t lru_loc_ = 0;
  bool bitmap_[elementCount]{};
  std::fstream file_{};
  filenameType filename_;
};

} // namespace StarryPurple_FileIO



#endif // FILE_STREAM_H