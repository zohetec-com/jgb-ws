#ifndef OBJECT_CALLBACK_H
#define OBJECT_CALLBACK_H

#include <jgb/config.h>
#include "connection_callback.h"
#include "server_callback.h"
#include <map>
#include <list>
#include <chrono>

namespace wsobj
{

// https://gist.github.com/dtoma/cc3d5b2dd4c03a25886adededcc1e3b9
static int64_t get_timestamp_us()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto epoch = now.time_since_epoch();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(epoch).count();
    return us;
}

class request
{
public:
    request(void* in, int len)
    {
        c = jgb::config_factory::create((char*) in, len);
        if(!c)
        {
            c = new jgb::config;
        }
    }

    ~request()
    {
        delete c;
    }

    jgb::config* c;

    std::string object()
    {
        std::string o;
        c->get("object", o);
        return o;
    }

    std::string method()
    {
        std::string m;
        c->get("method", m);
        return m;
    }
};

class response
{
public:
    jgb::config* c;

    response(int status = 0)
    {
        c = new jgb::config;
        c->create("status", status);
        c->create("time", get_timestamp_us());
    }

    void ok()
    {
        c->set("status", 200);
    }
};

class object_dispatch_callback;

class connection_context: public connection_callback
{
public:
    connection_context(struct lws* wsi): connection_callback(wsi)
    {
    }

    void on_recv(void *in, int len) override
    {
        object_dispatch_callback::get_instance()->process(*this, in, len);
    }

    void on_send() override
    {
        std::shared_ptr<response> resp = resps_.pop_front();
        std::string str = resp->c->to_string();
        send(str);
    }

    std::list<std::shared_ptr<response>> resps_;
};

class object_callback
{
public:
    virtual int process(connection_context& ctx, request& req, response& resp) = 0;
    int priority;
};

class object_dispatch_callback: public connection_callback_factory, public object_callback
{
public:

    int install(const std::string& name, object_callback* cb)
    {
        callbacks_[name].push_back(cb);
        callbacks_[name].sort([](object_callback* a, object_callback* b) {
            return a->priority < b->priority;
        });
        return 0;
    }

    static object_dispatch_callback* get_instance()
    {
        static object_dispatch_callback instance;
        return &instance;
    }

    connection_callback* create(struct lws* wsi) override
    {
        connection_context* ctx = new connection_context(wsi);
        return ctx;
    }

    int process(connection_context &ctx, void* in, int len)
    {
        request req(in, len);
        std::shared_ptr<response*> resp = std::make_shared<response>();
        int r = process(ctx, req, *resp);
        ctx.resps_.push_back(*resp);
        request_to_send(wsi_);
        return r;
    }

    int process(connection_context &ctx, request &req, response &resp) override
    {
        std::string o = req.object();

        auto it = callbacks_.find(o);
        if(it != callbacks_.end())
        {
            for(auto cb : it->second)
            {
                int ret = cb->process(ctx, req, resp);
                if(ret != 0)
                {
                    return ret;
                }
            }
        }

        return 0;
    }

    std::map<std::string,std::list<object_callback*>> callbacks_;
};

}

#endif // OBJECT_CALLBACK_H
