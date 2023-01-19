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
    return false; // FIXME:
}

void cowircd::user::on_read() throw()
{
    byte_buffer inbound(4096);
    ::ssize_t r = ::recv(this->get_fd(), inbound.raw_buffer(), inbound.raw_length(), 0);
    if (r <= 0)
    {
        if (r == 0)
        {
            std::cout << "Gracefully shutdown" << std::endl;
        }
        else
        {
            std::cerr << "what: " << std::strerror(errno) << std::endl;
        }
        this->worker->deregister_entry(this->get_fd());
        return;
    }
    inbound.raw_shrink(r);

    std::cout << "recv notified " << this->get_fd() << std::endl;
    ::send(this->get_fd(), inbound.raw_buffer(), inbound.raw_length(), 0); // echo for test
}

void cowircd::user::on_write() throw()
{
    std::cout << "send notified " << this->get_fd() << std::endl;
}