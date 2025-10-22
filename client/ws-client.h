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
            if(!req_)
            {
                if(req_str_)
                {
                    req_ = new ws::request((char*)req_str_, strlen(req_str_));
                }
            }

            if(req_)
            {
                send(*req_);
            }
            sent_ = false;
        }
    }

    virtual void on_recv(void *in, int len)
    {
        jgb_raw("%.*s\n", len, (char*)in);
    }

    ws_client(jgb::config* conf)
        : client_callback(conf),
        interval_(1000),
        sent_(false),
        req_(nullptr)
    {
    }

    static const char* req_str_;
    int interval_; // ms

private:
    bool sent_;
    ws::request* req_;
};

#endif // TEST_CLIENT_H
