// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include "socket_entry.hpp"

#include <uy_shared_ptr.hpp>

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
        void loop();

        void register_entry(const uy::shared_ptr<socket_entry>& entry);
        void deregister_entry(int fd);
    };
}
