// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#include "room.hpp"

#include "irc_message.hpp"
#include "server.hpp"
#include "user.hpp"

#include <uy_shared_ptr.hpp>

#include <vector>

cowircd::room::room(server& svr, const std::string& name)
    : svr(svr), name(name)
{
}

cowircd::room::~room()
{
}

void cowircd::room::join(const uy::shared_ptr<user>& usr)
{
    // TODO: prevent double join
    this->user_list.push_back(usr);
    // TODO: this->broadcast(irc_message("JOIN"));
}

void cowircd::room::quit(user* usr, const std::string& reason)
{
    for (std::vector<uy::shared_ptr<user> >::iterator it = this->user_list.begin(); it != this->user_list.end(); ++it)
    {
        if (it->get() == usr)
        {
            this->user_list.erase(it);
            static_cast<void>(reason);
            // TODO: this->broadcast(irc_message("QUIT"));
            break;
        }
    }
    // TODO: 아무도 남지 않은 방 처분
}

void cowircd::room::broadcast(const irc_message& msg) const
{
    // vector는 unstable하기 때문에 broadcast 중 erase가 호출되면 iterator가 invalidate 될 수 있으므로 copy를 만든다.
    //   아니 이 정도면 그냥 영문 주석인데?
    std::vector<uy::shared_ptr<user> > copy = this->user_list;
    for (std::vector<uy::shared_ptr<user> >::iterator it = copy.begin(); it != copy.end(); ++it)
    {
        const uy::shared_ptr<user>& usr = *it;
        usr->send_message(msg);
    }
}
