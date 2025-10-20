#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

#include "client_callback.h"
#include "message.h"

class test_client: public client_callback
{
public:

    virtual void on_session() override
    {
        jgb::sleep(1000);
        sent_ = true;
        request_to_send(wsi_);
    }

    virtual void on_send() override
    {
        if(sent_)
        {
            wsobj::request req("hello", "get", id_++);
            std::string str = req.to_string();
            send(str);
            sent_ = false;
        }
    }

    virtual void on_recv(void *in, int len)
    {
        jgb_raw("%.*s\n", len, (char*)in);
    }

    test_client(jgb::config* conf)
        : client_callback(conf)
    {
    }

private:
    bool sent_ = false;
    int64_t id_ = 1000;
};

#endif // TEST_CLIENT_H
