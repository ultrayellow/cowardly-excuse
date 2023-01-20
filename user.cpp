// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#include "user.hpp"

#include "irc_message.hpp"
#include "looper.hpp"
#include "socket_entry.hpp"

#include <sys/socket.h>
#include <sys/types.h>

#include <cerrno>
#include <cstring>
#include <string>

#include <iostream>

cowircd::user::user(const std::string& remote_addr, int remote_port, int fd)
    : socket_entry(fd), remote_addr(remote_addr), remote_port(remote_port), outbound()
{
}

cowircd::user::~user()
{
    std::cout << "유저 소멸: #" << this->get_fd() << " " << this->remote_addr << " : " << this->remote_port << std::endl;
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
                std::cout << "Gracefully shutdown" << std::endl;
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
            this->worker->deregister_entry(fd);
            return;
        }
        inbound.raw_shrink(r);

        this->process_byte_buffer(&inbound);
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
            this->worker->deregister_entry(fd);
            return;
        }
        this->outbound.remove(r);
    }
    this->outbound.discard();
}

void cowircd::user::send_message()
{
    this->do_write_message(NULL);
}

void cowircd::user::do_write(const unsigned char* buf, std::size_t len)
{
    this->outbound.put(buf, len);
    this->on_write();
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

void cowircd::user::do_write_string(void* arg)
{
    std::string& str = *static_cast<std::string*>(arg);
    str.append("\r\n");
    this->do_write(reinterpret_cast<const unsigned char*>(str.data()), sizeof(std::string::value_type) * str.size());
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
            std::cerr << "parse error: \"" << s << "\"" << std::endl;
        }
    }
    this->cumulative_lines.clear();
}

void cowircd::user::do_write_message(void*)
{
    // TODO: irc_message::to_string
}

void cowircd::user::process_message(void* arg)
{
    irc_message& msg = *static_cast<irc_message*>(arg);

    std::cout << std::endl;
    std::cout << "{" << std::endl;
    std::cout << "\tCOMMAND=\"" << msg.get_command() << "\"" << std::endl;
    if (msg.has_prefix())
    {
        std::cout << "\tPREFIX=\"" << msg.get_prefix() << "\"" << std::endl;
    }
    else
    {
        std::cout << "\tPREFIX=(null)" << std::endl;
    }
    std::cout << "\tPARAMS = [" << msg.size_param() << "] {" << std::endl;
    for (std::size_t i = 1; i <= msg.size_param(); i++)
    {
        std::cout << "\t\t[" << i << "] = \"" << msg[i] << "\"" << std::endl;
    }
    std::cout << "\t}" << std::endl;
    std::cout << "}" << std::endl;
}
