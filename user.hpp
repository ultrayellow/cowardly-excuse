// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include "byte_buffer.hpp"
#include "socket_entry.hpp"

#include <uy_shared_ptr.hpp>

#include <string>
#include <vector>

namespace cowircd
{
    class user : public socket_entry
    {
    private:
        std::string remote_addr;
        int remote_port;
        byte_buffer outbound;

    public:
        user(const std::string& remote_addr, int remote_port, int fd);
        ~user();

        bool is_readability_interested() const throw();
        bool is_writability_interested() const throw();
        void on_read() throw();
        void on_write() throw();

        void do_write(const unsigned char* buf, std::size_t len);

    private:
        // #1. LineBasedFrameDecoder
        byte_buffer cumulative;
        void process_byte_buffer(void*);

        // #2. StringEncoder
        void do_write_string(void*);

        // #3. IRCMessageCodecAdapter
        std::vector<std::string> cumulative_lines;
        void process_string_vector(void*);
        void do_write_message(void*);

        // #4. IRCHandler
        void process_message(void*);
    };
}
