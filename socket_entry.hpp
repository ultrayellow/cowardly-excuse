// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include <fcntl.h>
#include <unistd.h>

#include <cassert>

namespace cowircd
{
    class looper;

    class socket_entry
    {
    private:
        int fd;

    protected:
        looper* worker;

    public:
        socket_entry(int fd) : fd(fd) { assert(::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL) | O_NONBLOCK) == 0); }
        virtual ~socket_entry() { assert(::close(fd) == 0); }

        int get_fd() const throw() { return this->fd; }
        void set_worker(looper* worker) throw() { this->worker = worker; }
        virtual bool is_readability_interested() const throw() = 0;
        virtual bool is_writability_interested() const throw() = 0;
        virtual void on_read() throw() = 0;
        virtual void on_write() throw() = 0;

    private:
        socket_entry(const socket_entry&);
        socket_entry& operator=(const socket_entry&);
    };
}
