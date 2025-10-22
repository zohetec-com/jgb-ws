#ifndef TEST_OBJECT_CALLBACK_H
#define TEST_OBJECT_CALLBACK_H

#include "object_callback.h"

class hello_object_callback: public ws::object_callback
{
public:
    int process(ws::connection_context&, ws::request& req, ws::response& resp) override
    {
        jgb_debug("%s", req.to_string().c_str());
        resp.ok();
        return 0;
    }

    static hello_object_callback* get_instance()
    {
        static hello_object_callback instance;
        return &instance;
    }
};

#endif // TEST_OBJECT_CALLBACK_H
