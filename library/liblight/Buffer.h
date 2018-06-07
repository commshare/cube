#pragma once
#include <algorithm>
#include <vector>
#include <assert.h>
#include <string>
#include <stdint.h>

namespace light
{
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
    static const size_t kInitialSize = 8 * 1024;

    Buffer();

    void swap(Buffer& rhs);

    size_t readableBytes() const;

    size_t writableBytes() const;

    size_t prependableBytes() const;

    const char* peek() const;

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

    void append(const char* data, size_t len);

    void append(const void* data, size_t len);

    void ensureWritableBytes(size_t len);

    char* beginWrite();

    const char* beginWrite() const;

    void hasWritten(size_t len);

    void unwrite(size_t len);

    void prepend(const void* data, size_t len);

    size_t internalCapacity() const;

    void remove();

private:
    char* begin();

    const char* begin() const;

    void makeSpace(size_t len);

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};
}