#include "Buffer.h"

namespace light
{
char* Buffer::begin()
{
    return &*buffer_.begin();
}

const char* Buffer::begin() const
{
    return &*buffer_.begin();
}

void Buffer::makeSpace(size_t len)
{
    if (has_capacity(len)) {
        // move readable data to the front, make space inside buffer
        remove();
    }
    else {
        // FIXME: move readable data
        buffer_.resize(writerIndex_ + len);
    }
}

Buffer::Buffer()
    : buffer_(kCheapPrepend + kInitialSize),
    readerIndex_(kCheapPrepend),
    writerIndex_(kCheapPrepend)
{
    assert(readableBytes() == 0);
    assert(writableBytes() == kInitialSize);
    assert(prependableBytes() == kCheapPrepend);
}

void Buffer::swap(Buffer& rhs)
{
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
}

size_t Buffer::readableBytes() const
{
    return writerIndex_ - readerIndex_;
}

size_t Buffer::writableBytes() const
{
    return buffer_.size() - writerIndex_;
}

size_t Buffer::prependableBytes() const
{
    return readerIndex_;
}

const char* Buffer::peek() const
{
    return begin() + readerIndex_;
}

void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    if (len < readableBytes())
    {
        readerIndex_ += len;
    }
    else
    {
        retrieveAll();
    }
}

void Buffer::retrieveUntil(const char* end)
{
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
}

void Buffer::retrieveInt32()
{
    retrieve(sizeof(int32_t));
}

void Buffer::retrieveInt16()
{
    retrieve(sizeof(int16_t));
}

void Buffer::retrieveInt8()
{
    retrieve(sizeof(int8_t));
}

void Buffer::retrieveAll()
{
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
}

bool Buffer::has_capacity(size_t len)
{
    return writableBytes() + prependableBytes() >= len + kCheapPrepend;
}

std::string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());;
}

std::string Buffer::retrieveAsString(size_t len)
{
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

void Buffer::append(const char* data, size_t len)
{
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const void* data, size_t len)
{
    append(static_cast<const char*>(data), len);
}

void Buffer::ensureWritableBytes(size_t len)
{
    if (writableBytes() < len)
    {
        makeSpace(len);
    }
    assert(writableBytes() >= len);
}

char* Buffer::beginWrite()
{
    return begin() + writerIndex_;
}

const char* Buffer::beginWrite() const
{
    return begin() + writerIndex_;
}

void Buffer::hasWritten(size_t len)
{
    assert(len <= writableBytes());
    writerIndex_ += len;
}

void Buffer::unwrite(size_t len)
{
    assert(len <= readableBytes());
    writerIndex_ -= len;
}

void Buffer::prepend(const void* data, size_t len)
{
    assert(len <= prependableBytes());
    readerIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d + len, begin() + readerIndex_);
}

size_t Buffer::internalCapacity() const
{
    return buffer_.capacity();
}

void Buffer::remove()
{
    assert(kCheapPrepend < readerIndex_);
    size_t readable = readableBytes();
    std::copy(begin() + readerIndex_,
        begin() + writerIndex_,
        begin() + kCheapPrepend);
    readerIndex_ = kCheapPrepend;
    writerIndex_ = readerIndex_ + readable;
    assert(readable == readableBytes());
}

}