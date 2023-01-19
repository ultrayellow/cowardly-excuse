// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include "socket_entry.hpp"

#include <uy_shared_ptr.hpp>

#include <sys/select.h>

#include <map>

namespace cowircd
{
    class looper
    {
    public:
        typedef std::map<int, uy::shared_ptr<socket_entry> > dictionary_type;

    private:
        dictionary_type entries;

    public:
        int retrieve_fd(::fd_set& rd, ::fd_set& wr);
        void loop();
        void notify_fd(int r, int max_fd, ::fd_set& rd, ::fd_set& wr);

        void register_entry(const uy::shared_ptr<socket_entry>& entry);
        void deregister_entry(int fd);
    };
}
