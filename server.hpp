// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include "socket_entry.hpp"

#include <uy_shared_ptr.hpp>

#include <map>
#include <string>

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
        std::map<std::string, uy::shared_ptr<room> > room_dictionary;

    public:
        server(int fd, const server_config& config);
        ~server();
        bool listen();

        bool is_readability_interested() const throw();
        bool is_writability_interested() const throw();
        void on_read() throw();
        void on_write() throw();
        void on_close() throw();

        const uy::shared_ptr<room>& get_or_new_channel(const std::string& name);
        void remove_channel(const std::string& name);

    private:
        server(const server&);
        server& operator=(const server&);
    };
}
