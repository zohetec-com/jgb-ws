#ifndef OBJECT_CALLBACK_H
#define OBJECT_CALLBACK_H

#include <jgb/config.h>
#include "connection_callback.h"
#include "server_callback.h"

namespace wsobj
{

class connection_context: public connection_callback
{
public:
    connection_context(struct lws* wsi): connection_callback(wsi)
    {
    }

    virtual void on_recv(void *in, int len)
    {
        jgb::config* conf = jgb::config_factory::create((char*) in, len);
        delete conf;
    }
};

class request
{
    jgb::config c;

    std::string object()
    {
        std::string o;
        c.get("object", o);
        return o;
    }

    std::string method()
    {
        std::string m;
        c.get("method", m);
        return m;
    }
};

class response
{
    jgb::config c;
};

class object_callback
{
public:
    virtual int process(connection_callback& ctx, request& req, response& resp) = 0;
    int priority;
};

class object_callback_dispatcher: public connection_callback_factory, public object_callback
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

    static object_callback_dispatcher* get_instance()
    {
        static object_callback_dispatcher instance;
        return &instance;
    }

    connection_callback* create(struct lws* wsi) override
    {
        connection_context* ctx = new connection_context(wsi);
        return ctx;
    }

    int process(connection_callback &ctx, request &req, response &resp) override
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
