#ifndef FILE_STREAM_TPP
#define FILE_STREAM_TPP

#include "filestream.h"

using StarryPurple::FileExceptions;

template<size_t capacity>
StarryPurple::Fpointer<capacity>::Fpointer(){
  setnull();
}

template<size_t capacity>
StarryPurple::Fpointer<capacity>::Fpointer(nullptr_t) {
  setnull();
}


template<size_t capacity>
StarryPurple::Fpointer<capacity>::Fpointer(offsetType offset): offset_(offset) {}

template<size_t capacity>
void StarryPurple::Fpointer<capacity>::setnull() {
  offset_ = capacity;
}

template<size_t capacity>
bool StarryPurple::Fpointer<capacity>::isnull() const {
  return offset_ == capacity;
}

template<size_t capacity>
bool StarryPurple::Fpointer<capacity>::operator==(const Fpointer &other) const {
  return offset_ == other.offset_;
}

template<size_t capacity>
bool StarryPurple::Fpointer<capacity>::operator!=(const Fpointer &other) const {
  return !(*this == other);
}




template<class StorageType, class InfoType, size_t capacity>
StarryPurple::Fstream<StorageType, InfoType, capacity>::~Fstream() {
  if(file_.is_open())
    file_.close();
}

template<class StorageType, class InfoType, size_t capacity>
bool StarryPurple::Fstream<StorageType, InfoType, capacity>::open(const std::string &filename) {
  filename_ = filename;
  if(file_.is_open())
    throw FileExceptions("Opening unclosed file \"" + filename + "\"" );
  file_.open(filename, std::ios::binary | std::ios::in | std::ios::out);
  if(file_.is_open()) {
    // file already exist.
    // read in info.
    file_.seekg(0, std::ios::beg);
    file_.read(reinterpret_cast<char *>(&extra_info_), cExtraInfoSize);
    file_.read(reinterpret_cast<char *>(&lru_loc_), sizeof(offsetType));
    for(size_t i = 0; i < capacity; i++)
      file_.read(reinterpret_cast<char *>(&bitmap_[i]), sizeof(bool));
    return true;
  } else {
    // file doesn't initially exist.
    // create one and initialize it.
    std::ofstream outfile(filename);
    outfile.close();
    file_.open(filename, std::ios::binary | std::ios::in | std::ios::out);
    file_.seekp(0, std::ios::beg);
    extra_info_ = InfoType();
    file_.write(reinterpret_cast<const char *>(&extra_info_), cExtraInfoSize);
    lru_loc_ = 0;
    file_.write(reinterpret_cast<const char *>(&lru_loc_), sizeof(offsetType));
    for(size_t i = 0; i < capacity; i++) {
      bitmap_[i] = false;
      file_.write(reinterpret_cast<const char *>(&bitmap_[i]), sizeof(bool));
    }
    // no need to write that much at first.
    // since the lru_pos adds up 1 by 1, this write is unnecessary and much time_consuming.
    /*
    StorageType placeholder;
    for(size_t i = 0; i < capacity; i++)
      file_.write(reinterpret_cast<const char *>(&placeholder), cStorageSize);
      */
    return false;
  }
}

template<class StorageType, class InfoType, size_t capacity>
void StarryPurple::Fstream<StorageType, InfoType, capacity>::close() {
  if(!file_.is_open())
    throw FileExceptions("Closing file while no file is open");
  file_.seekp(0, std::ios::beg);
  file_.write(reinterpret_cast<const char *>(&extra_info_), cExtraInfoSize);
  file_.write(reinterpret_cast<const char *>(&lru_loc_), sizeof(offsetType));
  for(size_t i = 0; i < capacity; i++)
    file_.write(reinterpret_cast<const char *>(&bitmap_[i]), sizeof(bool));
  file_.close();
}

template<class StorageType, class InfoType, size_t capacity>
StarryPurple::Fpointer<capacity>
StarryPurple::Fstream<StorageType, InfoType, capacity>::allocate() {
  if(!file_.is_open())
    throw FileExceptions("Allocating storage while no file is open");
  offsetType loc = lru_loc_;
  while(loc < capacity && bitmap_[loc]) loc++;
  if(loc != capacity)
    lru_loc_ = loc;
  else {
    loc = 0;
    while(loc < lru_loc_ && bitmap_[loc]) loc++;
    if(loc != capacity)
      lru_loc_ = loc;
    else throw FileExceptions("Storage is full in file \"" + filename_ + "\"");
  }
  StorageType placeholder{};
  fpointer ptr{lru_loc_};
  bitmap_[lru_loc_] = true; // occupy the block before call of "write"
  write(placeholder, ptr);
  return ptr;
}


template<class StorageType, class InfoType, size_t capacity>
StarryPurple::Fpointer<capacity>
StarryPurple::Fstream<StorageType, InfoType, capacity>::allocate(const StorageType &data) {
  if(!file_.is_open())
    throw FileExceptions("Allocating storage while no file is open");
  offsetType loc = lru_loc_;
  while(loc < capacity && bitmap_[loc]) loc++;
  if(loc != capacity)
    lru_loc_ = loc;
  else {
    loc = 0;
    while(loc < lru_loc_ && bitmap_[loc]) loc++;
    if(loc != capacity)
      lru_loc_ = loc;
    else
      throw FileExceptions("Storage is full in file \"" + filename_ + "\"");
  }
  fpointer ptr{lru_loc_};
  bitmap_[lru_loc_] = true; // occupy the block before call of "write"
  write(data, ptr);
  return ptr;
}

template<class StorageType, class InfoType, size_t capacity>
void StarryPurple::Fstream<StorageType, InfoType, capacity>::free(const fpointer &ptr) {
  if(!file_.is_open())
    throw FileExceptions("Freeing storage while no file is open");
  const offsetType offset = ptr.offset_; // ??? why I can use it without friend class declaration?
  if(offset >= capacity)
    throw FileExceptions("Invalid reference in file \"" + filename_ + "\"");
  if(!bitmap_[offset])
    throw FileExceptions("Freeing unallocated storage in file \"" + filename_ + "\"");
  bitmap_[offset] = false;
}

template<class StorageType, class InfoType, size_t capacity>
void StarryPurple::Fstream<StorageType, InfoType, capacity>::read(StorageType &data, const fpointer &ptr) {
  if(!file_.is_open())
    throw FileExceptions("Reading storage while no file is open");
  const offsetType offset = ptr.offset_;
  if(offset >= capacity)
    throw FileExceptions("Invalid reference in file \"" + filename_ + "\"");
  if(!bitmap_[offset])
    throw FileExceptions("Reading unallocated storage in file \"" + filename_ + "\"");
  file_.seekg(cInfoSize + cStorageSize * offset, std::ios::beg);
  file_.read(reinterpret_cast<char *>(&data), cStorageSize);
}

template<class StorageType, class InfoType, size_t capacity>
void StarryPurple::Fstream<StorageType, InfoType, capacity>::write(const StorageType &data, const fpointer &ptr) {
  if(!file_.is_open())
    throw FileExceptions("Writing on storage while no file is open");
  const offsetType offset = ptr.offset_;
  if(offset >= capacity)
    throw FileExceptions("Invalid reference in file \"" + filename_ + "\"");
  if(!bitmap_[offset])
    throw FileExceptions("Writing unallocated storage in file \"" + filename_ + "\"");
  file_.seekp(cInfoSize + cStorageSize * offset, std::ios::beg);
  file_.write(reinterpret_cast<const char *>(&data), cStorageSize);
}

template<class StorageType, class InfoType, size_t capacity>
void StarryPurple::Fstream<StorageType, InfoType, capacity>::read_info(InfoType &info) {
  if(!file_.is_open())
    throw FileExceptions("Reading info while no file is open");
  info = extra_info_;
}

template<class StorageType, class InfoType, size_t capacity>
void StarryPurple::Fstream<StorageType, InfoType, capacity>::write_info(const InfoType &info) {
  if(!file_.is_open())
    throw FileExceptions("Writing on info while no file is open");
  extra_info_ = info;
}

#endif // FILE_STREAM_TPP