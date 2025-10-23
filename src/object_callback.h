#ifndef OBJECT_CALLBACK_H
#define OBJECT_CALLBACK_H

#include <jgb-ws/protocol_dispatch_callback.h>
#include <jgb-ws/wsapp.h>
#include <jgb-ws/message.h>
#include <map>
#include <list>

namespace ws
{

class object_dispatch_callback;

class connection_context: public connection_callback
{
public:
    connection_context(struct lws* wsi): connection_callback(wsi)
    {
    }

    void on_recv(void *in, int len) override;

    void on_send() override
    {
        if(!resps_.empty())
        {
            std::shared_ptr<response> resp = resps_.front();
            resps_.pop_front();
            std::string str = resp->c->to_string();
            jgb_debug("{ str = %s, len = %u }", str.c_str(), str.length());
            send(str);
        }
    }

    std::list<std::shared_ptr<response>> resps_;
};

class object_callback
{
public:
    virtual ~object_callback(){};
    virtual int process(connection_context& ctx, request& req, response& resp) = 0;
    int priority;
};

class object_dispatch_callback: public connection_callback_factory
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
        std::shared_ptr<response> resp = std::shared_ptr<response>(new response(req.id()));
        std::string o = req.object();
        auto it = callbacks_.find(o);
        int r;
        if(it != callbacks_.end())
        {
            for(auto cb : it->second)
            {
                // https://stackoverflow.com/questions/13575821/how-to-get-a-reference-to-an-object-having-shared-ptr-to-it
                r = cb->process(ctx, req, *resp);
                if(r != 0)
                {
                    break;
                }
            }
        }
        ctx.resps_.push_back(resp);
        request_to_send(ctx.wsi_);
        return 0;
    }

    std::map<std::string,std::list<object_callback*>> callbacks_;
};

}

#endif // OBJECT_CALLBACK_H
