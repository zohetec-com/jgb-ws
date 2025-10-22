#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

#include "client_callback.h"
#include "jgb/helper.h"
#include "message.h"

class ws_client: public client_callback
{
public:

    virtual void on_session() override
    {
        jgb::sleep(interval_);
        sent_ = true;
        request_to_send(wsi_);
    }

    void send(ws::request& req)
    {
        std::string str = req.to_string();
        client_callback::send(str);
    }

    virtual void on_send() override
    {
        if(sent_)
        {
            ws::request req("places", "read", 0);
            send(req);
            sent_ = false;
        }
    }

    virtual void on_recv(void *in, int len)
    {
        jgb_raw("%.*s\n", len, (char*)in);
    }

    ws_client(jgb::config* conf)
        : client_callback(conf),
        sent_(false),
        interval_(1000)
    {
    }

private:
    bool sent_;
    int interval_; // ms
};

#endif // TEST_CLIENT_H
