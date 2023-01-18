// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include "socket_entry.hpp"

#include <uy_shared_ptr.hpp>

namespace cowircd
{
    struct server_config
    {
        int port;
        int backlog;
    };

    class server : public socket_entry
    {
    private:
        const server_config& config;

    public:
        server(int fd, const server_config& config);
        bool listen();

        bool is_readability_interested() const throw();
        bool is_writability_interested() const throw();
        void on_read() throw();
        void on_write() throw();
    };
}