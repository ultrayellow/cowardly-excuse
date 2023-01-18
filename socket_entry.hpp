// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include <unistd.h>

namespace cowircd
{
    class looper;

    class socket_entry
    {
    private:
        int fd;
        looper* worker;

    public:
        socket_entry(int fd) : fd(fd) {}
        virtual ~socket_entry() { ::close(fd); }

        int get_fd() const throw() { return this->fd; }
        void set_worker(looper* worker) throw() { this->worker = worker; }
        virtual bool is_readability_interested() const throw() = 0;
        virtual bool is_writability_interested() const throw() = 0;
        virtual void on_read() throw() = 0;
        virtual void on_write() throw() = 0;
    };
}
