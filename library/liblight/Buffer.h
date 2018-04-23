// Copyright 2016, shaotang.shi.  All rights reserved.
// Author: shaotang.shi
//
// This is a public header file, it must only include public header files.

#pragma once
#include <algorithm>
#include <vector>
#include <assert.h>
#include <string>
#include <stdint.h>

namespace light
{
/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer
{
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 8*1024;

  Buffer();

  // implicit copy-ctor, move-ctor, dtor and assignment are fine
  // NOTE: implicit move-ctor is added in g++ 4.6

  void swap(Buffer& rhs);

  size_t readableBytes() const;

  size_t writableBytes() const;

  size_t prependableBytes() const;

  const char* peek() const;

  const char* findCRLF() const;

  const char* findCRLF(const char* start) const;

//  const char* findEOL() const;

//  const char* findEOL(const char* start) const;

  // retrieve returns void, to prevent
  // string str(retrieve(readableBytes()), readableBytes());
  // the evaluation of two functions are unspecified
  void retrieve(size_t len);

  void retrieveUntil(const char* end);

  void retrieveInt32();

  void retrieveInt16();

  void retrieveInt8();

  void retrieveAll();

  bool has_capacity(size_t len);
  std::string retrieveAllAsString();

  std::string retrieveAsString(size_t len);

  //StringPiece toStringPiece() const
  //{
  //  return StringPiece(peek(), static_cast<int>(readableBytes()));
  //}

  //void append(const StringPiece& str)
  //{
  //  append(str.data(), str.size());
  //}

  void append(const char* /*restrict*/ data, size_t len);

  void append(const void* /*restrict*/ data, size_t len);

  void ensureWritableBytes(size_t len);

  char* beginWrite();

  const char* beginWrite() const;

  void hasWritten(size_t len);

  void unwrite(size_t len);

  ///
  /// Append int32_t using network endian
  ///
  //void appendInt32(int32_t x)
  //{
  //  int32_t be32 = sockets::hostToNetwork32(x);
  //  append(&be32, sizeof be32);
  //}

  //void appendInt16(int16_t x)
  //{
  //  int16_t be16 = sockets::hostToNetwork16(x);
  //  append(&be16, sizeof be16);
  //}

  //void appendInt8(int8_t x)
  //{
  //  append(&x, sizeof x);
  //}

  ///
  /// Read int32_t from network endian
  ///
  /// Require: buf->readableBytes() >= sizeof(int32_t)
  //int32_t readInt32()
  //{
  //  int32_t result = peekInt32();
  //  retrieveInt32();
  //  return result;
  //}

  //int16_t readInt16()
  //{
  //  int16_t result = peekInt16();
  //  retrieveInt16();
  //  return result;
  //}

  //int8_t readInt8()
  //{
  //  int8_t result = peekInt8();
  //  retrieveInt8();
  //  return result;
  //}

  ///
  /// Peek int32_t from network endian
  ///
  /// Require: buf->readableBytes() >= sizeof(int32_t)
  //int32_t peekInt32() const
  //{
  //  assert(readableBytes() >= sizeof(int32_t));
  //  int32_t be32 = 0;
  //  ::memcpy(&be32, peek(), sizeof be32);
  //  return sockets::networkToHost32(be32);
  //}

  //int16_t peekInt16() const
  //{
  //  assert(readableBytes() >= sizeof(int16_t));
  //  int16_t be16 = 0;
  //  ::memcpy(&be16, peek(), sizeof be16);
  //  return sockets::networkToHost16(be16);
  //}

  //int8_t peekInt8() const
  //{
  //  assert(readableBytes() >= sizeof(int8_t));
  //  int8_t x = *peek();
  //  return x;
  //}

  ///
  /// Prepend int32_t using network endian
  ///
  //void prependInt32(int32_t x)
  //{
  //  int32_t be32 = sockets::hostToNetwork32(x);
  //  prepend(&be32, sizeof be32);
  //}

  //void prependInt16(int16_t x)
  //{
  //  int16_t be16 = sockets::hostToNetwork16(x);
  //  prepend(&be16, sizeof be16);
  //}

  //void prependInt8(int8_t x)
  //{
  //  prepend(&x, sizeof x);
  //}

  void prepend(const void* /*restrict*/ data, size_t len);

 //void shrink(size_t reserve)
 //{
 //  // FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
 //  Buffer other;
 //  other.ensureWritableBytes(readableBytes()+reserve);
 //  other.append(toStringPiece());
 //  swap(other);
 //}

  size_t internalCapacity() const;

  /// Read data directly into buffer.
  ///
  /// It may implement with readv(2)
  /// @return result of read(2), @c errno is saved
  //ssize_t readFd(int fd, int* savedErrno);
  void remove();
 private:

  char* begin();

  const char* begin() const;

  void makeSpace(size_t len);

 private:
  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;
  static const char kCRLF[];
};
}

