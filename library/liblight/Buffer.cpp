#include "Buffer.h"

char* light::Buffer::begin()
{
	return &*buffer_.begin();
}


void light::Buffer::makeSpace( size_t len )
{
	if (writableBytes() + prependableBytes() < len + kCheapPrepend)
	{
		// FIXME: move readable data
		buffer_.resize(writerIndex_+len);
	}
	else
	{
		// move readable data to the front, make space inside buffer
		remove();
	}
}

const char* light::Buffer::begin() const
{
	return &*buffer_.begin();
}

const char light::Buffer::kCRLF[] = "\r\n";
//const size_t light::Buffer::kCheapPrepend;
//const size_t light::Buffer::kInitialSize;

light::Buffer::Buffer()
	: buffer_(kCheapPrepend + kInitialSize),
	readerIndex_(kCheapPrepend),
	writerIndex_(kCheapPrepend)
{
	assert(readableBytes() == 0);
	assert(writableBytes() == kInitialSize);
	assert(prependableBytes() == kCheapPrepend);
}

void light::Buffer::swap( Buffer& rhs )
{
	buffer_.swap(rhs.buffer_);
	std::swap(readerIndex_, rhs.readerIndex_);
	std::swap(writerIndex_, rhs.writerIndex_);
}

size_t light::Buffer::readableBytes() const
{
	return writerIndex_ - readerIndex_;
}

size_t light::Buffer::writableBytes() const
{
	return buffer_.size() - writerIndex_;
}

size_t light::Buffer::prependableBytes() const
{
	return readerIndex_;
}

const char* light::Buffer::peek() const
{
	return begin() + readerIndex_;
}

const char* light::Buffer::findCRLF() const
{
	const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
	return crlf == beginWrite() ? NULL : crlf;
}


//const char* light::Buffer::findEOL() const
//{
//	const void* eol = memchr(peek(), '\n', readableBytes());
//	return static_cast<const char*>(eol);
//}


void light::Buffer::retrieve( size_t len )
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


void light::Buffer::retrieveUntil( const char* end )
{
	assert(peek() <= end);
	assert(end <= beginWrite());
	retrieve(end - peek());
}


void light::Buffer::retrieveInt32()
{
	retrieve(sizeof(int32_t));
}


void light::Buffer::retrieveInt16()
{
	retrieve(sizeof(int16_t));
}


void light::Buffer::retrieveInt8()
{
	retrieve(sizeof(int8_t));
}


void light::Buffer::retrieveAll()
{
	readerIndex_ = kCheapPrepend;
	writerIndex_ = kCheapPrepend;
}


bool light::Buffer::has_capacity(size_t len)
{
	return writableBytes() + prependableBytes() >= len + kCheapPrepend;
}

std::string light::Buffer::retrieveAllAsString()
{
	return retrieveAsString(readableBytes());;
}


std::string light::Buffer::retrieveAsString( size_t len )
{
	assert(len <= readableBytes());
	std::string result(peek(), len);
	retrieve(len);
	return result;
}


void light::Buffer::append( const char* /*restrict*/ data, size_t len )
{
	ensureWritableBytes(len);
	std::copy(data, data+len, beginWrite());
	hasWritten(len);
}


void light::Buffer::ensureWritableBytes( size_t len )
{
	if (writableBytes() < len)
	{
		makeSpace(len);
	}
	assert(writableBytes() >= len);
}


char* light::Buffer::beginWrite()
{
	return begin() + writerIndex_;
}


void light::Buffer::hasWritten( size_t len )
{
	assert(len <= writableBytes());
	writerIndex_ += len;
}


void light::Buffer::unwrite( size_t len )
{
	assert(len <= readableBytes());
	writerIndex_ -= len;
}


void light::Buffer::prepend( const void* /*restrict*/ data, size_t len )
{
	assert(len <= prependableBytes());
	readerIndex_ -= len;
	const char* d = static_cast<const char*>(data);
	std::copy(d, d+len, begin()+readerIndex_);
}


size_t light::Buffer::internalCapacity() const
{
	return buffer_.capacity();
}


void light::Buffer::remove()
{
	assert(kCheapPrepend < readerIndex_);
	size_t readable = readableBytes();
	std::copy(begin()+readerIndex_,
		begin()+writerIndex_,
		begin()+kCheapPrepend);
	readerIndex_ = kCheapPrepend;
	writerIndex_ = readerIndex_ + readable;
	assert(readable == readableBytes());
}

const char* light::Buffer::beginWrite() const
{
	return begin() + writerIndex_;
}

void light::Buffer::append( const void* /*restrict*/ data, size_t len )
{
	append(static_cast<const char*>(data), len);
}

//const char* light::Buffer::findEOL( const char* start ) const
//{
//	assert(peek() <= start);
//	assert(start <= beginWrite());
//	const void* eol = memchr(start, '\n', readableBytes());
//	return static_cast<const char*>(eol);
//}

const char* light::Buffer::findCRLF( const char* start ) const
{
	assert(peek() <= start);
	assert(start <= beginWrite());
	const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
	return crlf == beginWrite() ? NULL : crlf;
}
