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
        ++ count_;
        sent_ = (count_ % 2) + 1;
        request_to_send(wsi_);
    }

    virtual void on_send() override
    {
        if(sent_ == 1)
        {
            wsobj::request req("hello", "get", id_++);
            std::string str = req.to_string();
            send(str);

        }
        else if(sent_ == 2)
        {
            wsobj::request req("places", "read", id_++);
            std::string str = req.to_string();
            send(str);
        }
        sent_ = 0;
    }

    virtual void on_recv(void *in, int len)
    {
        jgb_raw("%.*s\n", len, (char*)in);
    }

    test_client(jgb::config* conf)
        : client_callback(conf),
        sent_(0),
        count_(0),
        id_(1000)
    {
    }

private:
    int sent_;
    int count_;
    int64_t id_;
};

#endif // TEST_CLIENT_H
