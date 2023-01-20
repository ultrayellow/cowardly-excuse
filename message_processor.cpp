// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#include "message_processor.hpp"

#include "room.hpp"
#include "server.hpp"
#include "user.hpp"

#include <uy_shared_ptr.hpp>

void cowircd::message_processor::run(server& svr, user& usr, irc_message& msg)
{
    const std::string& cmd = msg[0];
    const int params = msg.size_param();
    if (cmd == "JOIN")
    {
        if (params < 1)
        {
            usr.send_message(irc_message("MSG") << "뭔소리야?"
                                                << "입장하려면 채널 이름을 말해");
            return;
        }

        const uy::shared_ptr<room>& channel = svr.get_or_new_channel(msg[1]);
        usr.enter_channel(channel.get());
        channel->join(&usr);
        channel->broadcast(irc_message(usr.to_prefix(), "JOIN"), &usr);
    }
    else if (cmd == "PART")
    {
        // NOTE: 매개변수 개수가 부족하면 msg[]에서 예외를 던지고 있다. 예외는 비용이 비싸니까 조건 분기로 처리하는 것이 좋다.

        room* channel = usr.find_channel(msg[1]);
        channel->broadcast(irc_message(usr.to_prefix(), "PART"), &usr);
        channel->part(&usr);
        usr.leave_channel(channel);
    }
    else if (cmd == "ECHOME")
    {
        usr.send_message(msg);
    }
}
