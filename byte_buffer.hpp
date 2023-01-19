// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include <cstring>
#include <vector>

#include <cassert>

namespace cowircd
{
    class byte_buffer
    {
    public:
        typedef std::vector<unsigned char> container_type;
        typedef container_type::size_type size_type;
        typedef container_type::iterator iterator;

    private:
        container_type buffer;
        size_type position;

    public:
        inline byte_buffer() : buffer(), position() {}
        inline byte_buffer(size_type length) : buffer(length), position() {}

        inline unsigned char* raw_buffer() throw()
        {
            return this->buffer.data();
        }

        inline size_type raw_length() const throw()
        {
            return this->buffer.size();
        }

        inline void raw_shrink(size_type length)
        {
            assert(length <= this->buffer.size());

            this->buffer.resize(length);
        }

        inline const unsigned char* get() const throw()
        {
            return &this->buffer[this->position];
        }

        template <typename T>
        inline T get(const size_type offset = size_type()) const
        {
            assert(this->size() >= offset + sizeof(T));

            // Copy elision
            T t;
            std::memcpy(&t, this->get() + offset, sizeof(t));
            return t;
        }

        inline size_type size() const throw()
        {
            assert(this->buffer.size() >= this->position);

            return this->buffer.size() - this->position;
        }

        inline void put(const void* const data, const size_type size)
        {
            const unsigned char* const array = reinterpret_cast<const unsigned char*>(data);
            this->buffer.insert(this->buffer.end(), &array[0], &array[size]);
        }

        template <typename T>
        inline void put(const T& t)
        {
            unsigned char data[sizeof(t)];
            std::memcpy(&data, &t, sizeof(t));
            this->put(data, sizeof(t));
        }

        inline void remove(const size_type size) throw()
        {
            assert(this->size() >= size);

            this->position += size;
        }

        inline void discard() throw()
        {
            const iterator begin = this->buffer.begin();
            const iterator end = begin + this->position;
            this->buffer.erase(begin, end);
            this->position = size_type();
        }

    private:
        byte_buffer(const byte_buffer&);
        byte_buffer& operator=(const byte_buffer&);
    };
}
