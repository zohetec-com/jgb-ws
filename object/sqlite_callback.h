#ifndef SQLITE_CALLBACK_H
#define SQLITE_CALLBACK_H

#include "object_callback.h"

namespace ws
{

class sqlite_callback: public ws::object_callback
{
public:
    int process(ws::connection_callback& ctx, ws::request& req, ws::response& resp) override
    {
        jgb_mark();
        return 0;
    }

    static sqlite_callback* get_instance()
    {
        static sqlite_callback instance;
        return &instance;
    }
};

}

#endif // SQLITE_CALLBACK_H
