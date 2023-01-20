// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include "socket_entry.hpp"

#include <map>
#include <uy_shared_ptr.hpp>

namespace cowircd
{
    struct server_config
    {
        int port;
        int backlog;
    };

    class user;
    class room;

    class server : public socket_entry
    {
    private:
        const server_config& config;

    public:
        server(int fd, const server_config& config);
        ~server();
        bool listen();

        bool is_readability_interested() const throw();
        bool is_writability_interested() const throw();
        void on_read() throw();
        void on_write() throw();
        void on_close() throw();

    private:
        server(const server&);
        server& operator=(const server&);
    };
}
