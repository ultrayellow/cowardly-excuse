// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#include "server.hpp"

#include "looper.hpp"
#include "socket_entry.hpp"
#include "user.hpp"

#include <uy_shared_ptr.hpp>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <iostream>

cowircd::server::server(int fd, const server_config& config)
    : socket_entry(fd), config(config)
{
}

bool cowircd::server::listen()
{
    const int fd = this->get_fd();

    struct ::sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = ::htons(this->config.port);

    return ::bind(fd, &reinterpret_cast<struct ::sockaddr&>(sin), sizeof(sin)) == 0 &&
           ::listen(fd, this->config.backlog) == 0;
}

bool cowircd::server::is_readability_interested() const throw()
{
    return true;
}

bool cowircd::server::is_writability_interested() const throw()
{
    return false;
}

void cowircd::server::on_read() throw()
{
    struct ::sockaddr_in addr;
    ::socklen_t addr_len = sizeof(addr);
LABEL_RETRY:
    int child_fd = ::accept(this->get_fd(), &reinterpret_cast<struct ::sockaddr&>(addr), &addr_len);
    if (child_fd < 0)
    {
        switch (errno)
        {
        case EAGAIN:
            return;

        case EINTR:
            goto LABEL_RETRY;

        case ECONNABORTED:
            std::cerr << "accept 실패: 연결이 중단되었음." << std::endl;
            break;

        case EMFILE:
            std::cerr << "accept 실패: 프로세스가 사용할 수 있는 FD가 고갈됨." << std::endl;
            break;

        case ENFILE:
            std::cerr << "accept 실패: 시스템이 사용할 수 있는 FD가 고갈됨." << std::endl;
            break;

        case ENOBUFS:
        case ENOMEM:
            std::cerr << "accept 실패: 메모리 여유가 충분하지 않음." << std::endl;
            break;

        case EPERM:
            std::cerr << "accept 실패: 방화벽이 연결을 금지했음." << std::endl;
            break;

        default:
            std::cerr << "accept 실패: 예상하지 못한 오류. 복구할 수 없음: " << std::strerror(errno) << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
    else
    {
        std::string remote_addr = ::inet_ntoa(addr.sin_addr);
        int remote_port = ::ntohs(addr.sin_port);

        std::cout << "accept 성공: #" << child_fd << " " << remote_addr << " : " << remote_port << std::endl;

        uy::shared_ptr<user> child = uy::make_shared<user>(remote_addr, remote_port, child_fd);
        this->worker->register_entry(child);
    }
}

void cowircd::server::on_write() throw()
{
    assert(false);
}
