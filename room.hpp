// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include "irc_message.hpp"

#include <uy_shared_ptr.hpp>

#include <vector>

namespace cowircd
{
    class server;
    class user;

    class room
    {
    private:
        server& svr;
        std::string name;
        std::vector<uy::shared_ptr<user> > user_list;

    public:
        room(server& svr, const std::string& name);
        ~room();

        void join(const uy::shared_ptr<user>& usr);
        void quit(user* usr, const std::string& reason);

        void broadcast(const irc_message& msg) const;

    private:
        room(const room&);
        room& operator=(const room&);
    };
}
