// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#include "looper.hpp"

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <map>

#include <iostream>

int cowircd::looper::retrieve_fd(::fd_set& rd, ::fd_set& wr)
{
    int max_fd = 0;

    FD_ZERO(&rd);
    FD_ZERO(&wr);

    for (dictionary_type::const_iterator it = this->entries.begin(); it != this->entries.end(); ++it)
    {
        int fd = it->first;
        const uy::shared_ptr<socket_entry>& val = it->second;

        if (val->is_readability_interested())
        {
            FD_SET(fd, &rd);
        }
        if (val->is_writability_interested())
        {
            FD_SET(fd, &wr);
        }
        max_fd = std::max(fd + 1, max_fd);
    }

    return max_fd;
}

void cowircd::looper::loop()
{
    for (;;)
    {
        ::fd_set rd;
        ::fd_set wr;

        int max_fd = this->retrieve_fd(rd, wr);

    LABEL_RETRY:
        int r = ::select(max_fd, &rd, &wr, NULL, NULL);
        if (r < 0)
        {
            switch (errno)
            {
            case EINTR:
                goto LABEL_RETRY;

            case EBADF:
                std::cerr << "select 실패: 이미 닫힌 FD가 있음. 1초 후 FD를 다시 수집하고 재시도 함." << std::endl;
                ::sleep(1);
                continue;

            case ENOMEM:
                std::cerr << "select 실패: 메모리 여유가 충분하지 않음. 5초 후 재시도 함." << std::endl;
                ::sleep(5);
                goto LABEL_RETRY;

            default:
                std::cerr << "select 실패: 예상하지 못한 오류. 복구할 수 없음: " << std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
                break;
            }
            break;
        }

        if (r != 0)
        {
            this->notify_fd(r, max_fd, rd, wr);
        }
    }
}

void cowircd::looper::notify_fd(int r, int max_fd, ::fd_set& rd, ::fd_set& wr)
{
    for (int fd = 0; fd < max_fd; fd++)
    {
        const bool recv_available = FD_ISSET(fd, &rd);
        const bool send_available = FD_ISSET(fd, &wr);
        if (recv_available || send_available)
        {
            dictionary_type::const_iterator it = this->entries.find(fd);
            if (it == this->entries.end())
            {
                continue;
            }

            const uy::shared_ptr<socket_entry>& val = it->second;
            if (send_available)
            {
                val->on_write();
            }
            if (recv_available)
            {
                val->on_read();
            }

            if (--r == 0)
            {
                break;
            }
        }
    }
}

void cowircd::looper::register_entry(const uy::shared_ptr<socket_entry>& entry)
{
    this->entries.insert(std::make_pair(entry->get_fd(), entry));
    entry->set_worker(this);
}

void cowircd::looper::deregister_entry(int fd)
{
    this->entries.erase(fd);
}
