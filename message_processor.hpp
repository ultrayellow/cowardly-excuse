// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

namespace cowircd
{
    class server;
    class user;
    class irc_message;

    struct message_processor
    {
        static void run(server&, user&, irc_message&);

    private:
        message_processor();
    };
}
