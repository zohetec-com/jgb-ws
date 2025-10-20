#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

#include "client_callback.h"
#include "message.h"

class test_client: public client_callback
{
public:

    virtual void on_send() override
    {
        wsobj::request req("hello", "get");
        std::string str = req.to_string();
        send(str);
    }

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
