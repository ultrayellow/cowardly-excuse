// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#include "user.hpp"

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
    return false; // FIXME: enable if final-outbound-buffer is not empty
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

        // NOTE: please fully consume inbound
        std::cout << "recv notified " << fd << std::endl;
        ::send(fd, inbound.raw_buffer(), inbound.raw_length(), 0); // echo for test
    }
}

void cowircd::user::on_write() throw()
{
    const int fd = this->get_fd();
    std::cout << "send notified " << fd << std::endl;
}
