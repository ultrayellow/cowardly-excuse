// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include "byte_buffer.hpp"
#include "socket_entry.hpp"

#include <uy_shared_ptr.hpp>

#include <string>

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
    };
}
