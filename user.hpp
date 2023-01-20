// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include "byte_buffer.hpp"
#include "irc_message.hpp"
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

        void send_message(const irc_message& msg);

    private:
        void do_write(const void* buf, std::size_t len);
        void do_flush();
        void do_write_and_flush(const void* buf, std::size_t len);

        // #1. LineBasedFrameDecoder
        byte_buffer cumulative;
        void process_byte_buffer(void*);

        // #2. StringEncoder
        void do_write_string(const void*);

        // #3. IRCMessageCodecAdapter
        std::vector<std::string> cumulative_lines;
        void process_string_vector(void*);
        void do_write_message(const void*);

        // #4. IRCHandler
        void process_message(void*);

    private:
        user(const user&);
        user& operator=(const user&);
    };
}
