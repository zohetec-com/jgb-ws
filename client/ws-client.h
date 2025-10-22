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
            if(loop_ || !count_)
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
            }
            sent_ = false;
        }
    }

    virtual void on_recv(void *in, int len)
    {
        jgb_raw("%.*s\n", len, (char*)in);
        ws::message msg(in, len);
        if(msg.is_response())
        {
            ++ count_;
        }
    }

    ws_client(jgb::config* conf)
        : client_callback(conf),
        req_str_(nullptr),
        interval_(1000),
        sent_(false),
        req_(nullptr),
        loop_(false),
        count_(0)
    {
        conf->get("loop", loop_);
        conf->get("req", &req_str_);
        conf->get("interval", interval_);
        conf->bind("count", &count_);
        jgb_debug("{ loop = %d, req = %s, interval = %d }", loop_, req_str_ ? req_str_ : "", interval_);
    }

    ~ws_client()
    {
        delete req_;
    }

private:
    const char* req_str_;
    int interval_; // ms
    bool sent_;
    ws::request* req_;
    bool loop_;
    int count_;
};

#endif // TEST_CLIENT_H
