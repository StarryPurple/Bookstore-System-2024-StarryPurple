#include "FileStream.h"

using StarryPurple::FileExceptions;

namespace StarryPurple {

template<class StorageType, size_t elementCount>
Fpointer<StorageType, elementCount>::Fpointer(): offset_(elementCount) {}

template<class StorageType, size_t elementCount>
Fpointer<StorageType, elementCount>::Fpointer(offsetType offset): offset_(offset) {}

template<class StorageType, size_t elementCount>
Fstream<StorageType, elementCount>::~Fstream() {
  if(file_.is_open())
    file_.close();
}

template<class StorageType, size_t elementCount>
void Fstream<StorageType, elementCount>::open(const filenameType &filename) {
  filename_ = filename;
  if(file_.is_open())
    throw FileExceptions("Opening unclosed file \"" + filename + "\"" );
  file_.open(filename, std::ios::binary | std::ios::in | std::ios::out);
  if(file_.is_open()) {
    // file already exist.
    // read in info.
    file_.seekg(0, std::ios::beg);
    file_.read(reinterpret_cast<char *>(&lru_loc_), sizeof(offsetType));
    for(size_t i = 0; i < elementCount; i++)
      file_.read(reinterpret_cast<char *>(&bitmap_[i]), sizeof(bool));
  } else {
    // file doesn't initially exist.
    // create one and initialize it.
    std::ofstream outfile(filename);
    outfile.close();
    file_.open(filename, std::ios::binary | std::ios::in | std::ios::out);
    file_.seekp(0, std::ios::beg);
    lru_loc_ = 0;
    file_.write(reinterpret_cast<const char *>(&lru_loc_), sizeof(offsetType));
    for(size_t i = 0; i < elementCount; i++) {
      bitmap_[i] = false;
      file_.write(reinterpret_cast<const char *>(&bitmap_[i]), sizeof(bool));
    }
    StorageType placeholder;
    for(size_t i = 0; i < elementCount; i++)
      file_.write(reinterpret_cast<const char *>(&placeholder), cStorageSize);
  }
}

template<class StorageType, size_t elementCount>
void Fstream<StorageType, elementCount>::close() {
  if(!file_.is_open())
    throw FileExceptions("Closing file while no file is open");
  file_.seekp(0, std::ios::beg);
  file_.write(reinterpret_cast<const char *>(&lru_loc_), sizeof(offsetType));
  for(size_t i = 0; i < elementCount; i++)
    file_.write(reinterpret_cast<const char *>(&bitmap_[i]), sizeof(bool));
  file_.close();
}

template<class StorageType, size_t elementCount>
typename Fstream<StorageType, elementCount>::fpointer
Fstream<StorageType, elementCount>::allocate() {
  if(!file_.is_open())
    throw FileExceptions("Allocating storage while no file is open");
  size_t loc = lru_loc_;
  while(loc < elementCount && bitmap_[loc]) loc++;
  if(loc != elementCount)
    lru_loc_ = loc;
  else {
    loc = 0;
    while(loc < lru_loc_ && bitmap_[loc]) loc++;
    if(loc != elementCount)
      lru_loc_ = loc;
    else
      throw FileExceptions("Storage is full in file \"" + filename_ + "\"");
  }
  StorageType placeholder{};
  fpointer ptr{lru_loc_};
  write(placeholder, ptr);
  bitmap_[lru_loc_] = true;
  return ptr;
}


template<class StorageType, size_t elementCount>
typename Fstream<StorageType, elementCount>::fpointer
Fstream<StorageType, elementCount>::allocate(const StorageType &data) {
  if(!file_.is_open())
    throw FileExceptions("Allocating storage while no file is open");
  size_t loc = lru_loc_;
  while(loc < elementCount && bitmap_[loc]) loc++;
  if(loc != elementCount)
    lru_loc_ = loc;
  else {
    loc = 0;
    while(loc < lru_loc_ && bitmap_[loc]) loc++;
    if(loc != elementCount)
      lru_loc_ = loc;
    else
      throw FileExceptions("Storage is full in file \"" + filename_ + "\"");
  }
  fpointer ptr(lru_loc_);
  bitmap_[lru_loc_] = true; // occupy it before call of "write"
  write(data, ptr);
  return ptr;
}

template<class StorageType, size_t elementCount>
void Fstream<StorageType, elementCount>::free(const fpointer &ptr) {
  if(!file_.is_open())
    throw FileExceptions("Freeing storage while no file is open");
  const offsetType offset = ptr.offset_; // ??? why I can use it without friend class declaration?
  if(offset >= elementCount)
    throw FileExceptions("Invalid reference in file \"" + filename_ + "\"");
  if(!bitmap_[offset])
    throw FileExceptions("Freeing unallocated storage in file \"" + filename_ + "\"");
  bitmap_[offset] = false;
}

template<class StorageType, size_t elementCount>
void Fstream<StorageType, elementCount>::read(StorageType &data, const fpointer &ptr) {
  if(!file_.is_open())
    throw FileExceptions("Reading storage while no file is open");
  const offsetType offset = ptr.offset_;
  if(offset >= elementCount)
    throw FileExceptions("Invalid reference in file \"" + filename_ + "\"");
  if(!bitmap_[offset])
    throw FileExceptions("Reading unallocated storage in file \"" + filename_ + "\"");
  file_.seekg(cInfoSize + cStorageSize * offset, std::ios::beg);
  file_.read(reinterpret_cast<char *>(&data), cStorageSize);
}

template<class StorageType, size_t elementCount>
void Fstream<StorageType, elementCount>::write(const StorageType &data, const fpointer &ptr) {
  if(!file_.is_open())
    throw FileExceptions("Writing on storage while no file is open");
  const offsetType offset = ptr.offset_;
  if(offset >= elementCount)
    throw FileExceptions("Invalid reference in file \"" + filename_ + "\"");
  if(!bitmap_[offset])
    throw FileExceptions("Reading unallocated storage in file \"" + filename_ + "\"");
  file_.seekp(cInfoSize + cStorageSize * offset, std::ios::beg);
  file_.write(reinterpret_cast<const char *>(&data), cStorageSize);
}


} // namespace StarryPurple