// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include "irc_message.hpp"

#include <cstdlib>
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
        std::vector<user*> user_list;

    public:
        room(server& svr, const std::string& name);
        ~room();

        const std::string& get_name() const throw();

        void join(user* usr);
        void part(user* usr);

        void broadcast(const irc_message& msg, user* except = NULL) const;

    private:
        room(const room&);
        room& operator=(const room&);
    };
}
