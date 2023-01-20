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

static void _start(cowircd::looper& worker, cowircd::server_group& group, int group_number, int port)
{
    cowircd::server_config cfg;
    cfg.port = port;
    cfg.backlog = 10;

    const int server_fd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0 || ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, "\1\1\1\1\1\1\1\1", sizeof(int)) < 0)
    {
        throw std::runtime_error(::strerror(errno));
    }

    uy::shared_ptr<cowircd::server> server = uy::make_shared<cowircd::server>(server_fd, cfg, group);
    if (!server->listen())
    {
        throw std::runtime_error(::strerror(errno));
    }

    worker.register_entry(server);
    std::cout << "[DEBUG] " << cfg.port << "번 포트에서 " << group_number << "번 그룹에 대한 서버 시작" << std::endl;
}

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

    cowircd::looper worker;

    cowircd::server_group group;
    for (int i = 0; i < 3; i++)
    {
        _start(worker, group, 1, port + i);
    }

    cowircd::server_group group2;
    for (int i = 10; i < 13; i++)
    {
        _start(worker, group2, 2, port + i);
    }

    worker.loop();

    return EXIT_SUCCESS;
}
