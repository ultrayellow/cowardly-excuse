// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#include "user.hpp"

#include "irc_message.hpp"
#include "looper.hpp"
#include "message_processor.hpp"
#include "room.hpp"
#include "server.hpp"
#include "socket_entry.hpp"

#include <sys/socket.h>
#include <sys/types.h>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

#include <iostream>

cowircd::user::user(const std::string& remote_addr, int remote_port, server& svr, int fd)
    : socket_entry(fd), remote_addr(remote_addr), remote_port(remote_port), outbound(), closed(), svr(svr)
{
    this->quit_reason = DEFAULT_DISCONNECT_REASON;
}

cowircd::user::~user()
{
    std::cout << "[DEBUG] 유저 소멸: #" << this->get_fd() << " " << this->remote_addr << " : " << this->remote_port << std::endl;
}

bool cowircd::user::is_readability_interested() const throw()
{
    return true;
}

bool cowircd::user::is_writability_interested() const throw()
{
    return this->outbound.size() != 0;
}

void cowircd::user::on_read() throw()
{
    const int fd = this->get_fd();
    byte_buffer inbound(4096);
    for (;;)
    {
        ::ssize_t r = ::recv(fd, inbound.raw_buffer(), inbound.raw_length(), 0);
        if (r <= 0)
        {
            if (r == 0)
            {
                std::cout << "[DEBUG] Gracefully shutdown" << std::endl;
            }
            else
            {
                switch (errno)
                {
                case EAGAIN:
                    return;

                case EINTR:
                    continue;

                default:
                    std::cerr << "recv 실패: 예상하지 못한 오류. 복구할 수 없음: " << std::strerror(errno) << std::endl;
                    break;
                }
            }
            this->disconnect();
            return;
        }
        inbound.raw_shrink(r);

        try
        {
            this->process_byte_buffer(&inbound);
        }
        catch (std::exception& e)
        {
            std::cerr << "recv 후 처리 실패: 복구를 시도하지 않음: " << e.what() << std::endl;
            this->disconnect();
            return;
        }
    }
}

void cowircd::user::on_write() throw()
{
    const int fd = this->get_fd();
    for (;;)
    {
        const unsigned char* buf = this->outbound.get();
        std::size_t len = this->outbound.size();
        if (len == 0)
        {
            break;
        }
        ::ssize_t r = ::send(fd, buf, len, MSG_NOSIGNAL);
        if (r < 0)
        {
            switch (errno)
            {
            case EAGAIN:
                // is_writability_interested에 의해 신호 대기가 활성화 됨.
                return;

            case EINTR:
                continue;

            case ECONNRESET:
                std::cerr << "send 실패: 상대방이 연결을 끊었음." << std::endl;
                break;

            case ENOMEM:
                std::cerr << "send 실패: 가용 메모리가 없음." << std::endl;
                break;

            case EPIPE:
                std::cerr << "send 실패: SIGPIPE" << std::endl;
                break;

            default:
                std::cerr << "send 실패: 예상하지 못한 오류. 복구할 수 없음: " << std::strerror(errno) << std::endl;
                break;
            }
            this->outbound.remove(len);
            this->outbound.discard();
            this->disconnect();
            return;
        }
        this->outbound.remove(r);
    }
    this->outbound.discard();
}

void cowircd::user::on_close() throw()
{
    if (this->closed)
    {
        return;
    }
    this->closed = true;

    std::vector<room*> move;
    this->room_list.swap(move);
    // 이제 room_list는 clear 되었다.
    for (std::vector<room*>::iterator it = move.begin(); it != move.end(); ++it)
    {
        room* channel = *it;
        channel->broadcast(irc_message(this->to_prefix(), "QUIT") << this->quit_reason);
        channel->part(this);
    }
}

void cowircd::user::send_message(const irc_message& msg)
{
    this->do_write_message(&msg);
}

void cowircd::user::disconnect()
{
    this->worker->deregister_entry(this->get_fd());
}

void cowircd::user::do_write(const void* buf, std::size_t len)
{
    this->outbound.put(buf, len);
}

void cowircd::user::do_flush()
{
    this->on_write();
}

void cowircd::user::do_write_and_flush(const void* buf, std::size_t len)
{
    this->do_write(buf, len);
    this->do_flush();
}

void cowircd::user::process_byte_buffer(void* arg)
{
    byte_buffer& inbound = *static_cast<byte_buffer*>(arg);
    byte_buffer::size_type remaining = this->cumulative.size();
    this->cumulative.put(inbound.get(), inbound.size());
    inbound.remove(inbound.size());

    std::vector<std::string> lines;
    const unsigned char* buf = this->cumulative.get();
    byte_buffer::size_type begin = 0;
    bool cr = false;
    for (byte_buffer::size_type i = remaining; i < this->cumulative.size(); i++)
    {
        if (cr && buf[i] == '\n')
        {
            lines.push_back(std::string(&buf[begin], &buf[i - 1]));
            begin = i + 1;
        }
        else if (i - begin > 512)
        {
            // NOTE: 이 정도 길이를 CR-LF 없이 보내는건 규약 위반 아니냐?
        }

        cr = buf[i] == '\r';
    }
    this->cumulative.remove(begin);

    if (!lines.empty())
    {
        this->process_string_vector(&lines);
    }
    this->cumulative.discard();
}

void cowircd::user::do_write_string(const void* arg)
{
    const std::string& str = *static_cast<const std::string*>(arg);
    this->do_write(str.data(), sizeof(std::string::value_type) * str.size());
    this->do_write_and_flush("\r\n", 2);
}

void cowircd::user::process_string_vector(void* arg)
{
    std::vector<std::string>& lines = *static_cast<std::vector<std::string>*>(arg);
    this->cumulative_lines.insert(this->cumulative_lines.end(), lines.begin(), lines.end());
    lines.clear();

    for (std::size_t i = 0; i < this->cumulative_lines.size(); i++)
    {
        const std::string& s = this->cumulative_lines[i];
        irc_message msg;
        if (irc_message::parse(s, msg))
        {
            this->process_message(&msg);
        }
        else
        {
            std::cerr << "[DEBUG] parse error: \"" << s << "\"" << std::endl;
        }
    }
    this->cumulative_lines.clear();
}

void cowircd::user::do_write_message(const void* arg)
{
    const irc_message& msg = *static_cast<const irc_message*>(arg);
    std::string str = msg.to_string();
    this->do_write_string(&str);
}

void cowircd::user::process_message(void* arg)
{
    irc_message& msg = *static_cast<irc_message*>(arg);

    std::cout << "[DEBUG]" << std::endl
              << msg.to_pretty_string() << std::endl;

    message_processor::run(this->svr, *this, msg);
}

std::string cowircd::user::to_prefix() const
{
    std::ostringstream oss;
    oss << "unknown" << this->remote_port << "@" << this->remote_addr;
    return oss.str();
}

void cowircd::user::enter_channel(room* channel)
{
    this->room_list.push_back(channel);
    std::vector<room*>::iterator it = std::find(this->room_list.begin(), this->room_list.end(), channel);
    if (it == this->room_list.end())
    {
        this->room_list.push_back(channel);
    }
}

cowircd::room* cowircd::user::find_channel(const std::string& name)
{
    for (std::vector<room*>::iterator it = this->room_list.begin(); it != this->room_list.end(); ++it)
    {
        room* channel = *it;
        if (channel->get_name() == name)
        {
            return channel;
        }
    }

    return NULL;
}

void cowircd::user::leave_channel(room* channel)
{
    std::vector<room*>::iterator it = std::find(this->room_list.begin(), this->room_list.end(), channel);
    if (it != this->room_list.end())
    {
        this->room_list.erase(it);
    }
}
