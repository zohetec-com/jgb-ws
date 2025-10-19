#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

#include "client_callback.h"

class test_client: public client_callback
{
public:

    virtual void on_recv(void *in, int len)
    {
        jgb_raw("%.*s\n", len, (char*)in);
    }

    test_client(jgb::config* conf)
        : client_callback(conf)
    {
    }
};

#endif // TEST_CLIENT_H
