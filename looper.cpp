#include "looper.hpp"

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <map>

void cowircd::looper::loop()
{
    for (;;)
    {
        int max_fd = 0;

        ::fd_set rd;
        ::fd_set wr;
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
            max_fd = std::max(fd, max_fd);
        }

    LABEL_RETRY:
        int r = ::select(max_fd, &rd, &wr, NULL, NULL);
        if (r < 0)
        {
            if (errno == EINTR)
            {
                goto LABEL_RETRY;
            }
            break;
        }

        if (r != 0)
        {
            for (int fd = 0; fd < max_fd; fd++)
            {
                const bool recv_available = FD_ISSET(fd, &rd);
                const bool send_available = FD_ISSET(fd, &wr);
                if (recv_available || send_available)
                {
                    const uy::shared_ptr<socket_entry>& val = this->entries[fd];
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
    }
}

void cowircd::looper::register_entry(uy::shared_ptr<socket_entry> entry)
{
    this->entries.insert(std::make_pair(entry->get_fd(), entry));
}

void cowircd::looper::deregister_entry(int fd)
{
    this->entries.erase(fd);
}
