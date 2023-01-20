// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#include "room.hpp"

#include "irc_message.hpp"
#include "server.hpp"
#include "user.hpp"

#include <algorithm>
#include <vector>

#include <iostream>

cowircd::room::room(server& svr, const std::string& name)
    : svr(svr), name(name)
{
    std::cout << "[DEBUG] 방 생성 " << this->name << std::endl;
}

cowircd::room::~room()
{
    std::cout << "[DEBUG] 방 소멸 " << this->name << std::endl;
}

const std::string& cowircd::room::get_name() const throw()
{
    return this->name;
}

void cowircd::room::join(user* usr)
{
    std::vector<user*>::iterator it = std::find(this->user_list.begin(), this->user_list.end(), usr);
    if (it == this->user_list.end())
    {
        this->user_list.push_back(usr);
    }
}

void cowircd::room::part(user* usr)
{
    std::vector<user*>::iterator it = std::find(this->user_list.begin(), this->user_list.end(), usr);
    if (it != this->user_list.end())
    {
        this->user_list.erase(it);
    }

    if (this->user_list.empty())
    {
        this->svr.remove_channel(this->name);
        // 이제 room에 대한 shared_count가 모두 소진되었으므로 delete this된 것이나 다름이 없다.
    }
}

void cowircd::room::broadcast(const irc_message& msg, user* except) const
{
    // vector는 unstable하기 때문에 broadcast 중 erase가 호출되면 iterator가 invalidate 될 수 있으므로 copy를 만든다.
    //   아니 이 정도면 그냥 영문 주석인데?
    std::vector<user*> copy = this->user_list;
    for (std::vector<user*>::iterator it = copy.begin(); it != copy.end(); ++it)
    {
        user* usr = *it;
        if (usr != except)
        {
            usr->send_message(msg);
        }
    }
}
