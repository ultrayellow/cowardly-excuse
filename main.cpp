// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#include "looper.hpp"
#include "server.hpp"

#include <uy_shared_ptr.hpp>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "포트를 넣어라." << std::endl;
        return EXIT_FAILURE;
    }

    const int port = std::atoi(argv[1]);
    if (port <= 0 || port >= 65536)
    {
        std::cerr << "포트 넣어준건 고마운데 \'" << port << "\'번 말고 1에서 65535 사이의 숫자로 고쳐라." << std::endl;
        return EXIT_FAILURE;
    }

    cowircd::server_config cfg;
    cfg.port = port;
    cfg.backlog = 10;

    const int server_fd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0 || ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, "\1\1\1\1\1\1\1\1", sizeof(int)) < 0)
    {
        throw std::runtime_error(::strerror(errno));
    }

    uy::shared_ptr<cowircd::server> server = uy::make_shared<cowircd::server>(server_fd, cfg);
    if (!server->listen())
    {
        throw std::runtime_error(::strerror(errno));
    }

    cowircd::looper worker;
    worker.register_entry(server);
    worker.loop();

    return EXIT_SUCCESS;
}