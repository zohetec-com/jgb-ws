#ifndef TEST_OBJECT_CALLBACK_H
#define TEST_OBJECT_CALLBACK_H

#include "object_callback.h"

class test_object_callback: public wsobj::object_callback
{
public:
    int process(wsobj::connection_callback& ctx, wsobj::request& req, wsobj::response& resp) override
    {
        jgb_mark();
        resp.ok();
        return 0;
    }
};

#endif // TEST_OBJECT_CALLBACK_H
