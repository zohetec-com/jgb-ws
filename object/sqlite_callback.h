#ifndef SQLITE_CALLBACK_H
#define SQLITE_CALLBACK_H

#include "object_callback.h"

namespace wsobj
{

class sqlite_callback: public wsobj::object_callback
{
public:
    int process(wsobj::connection_callback& ctx, wsobj::request& req, wsobj::response& resp) override
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
