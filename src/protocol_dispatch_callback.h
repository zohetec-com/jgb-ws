#ifndef SERVER_CALLBACK_H
#define SERVER_CALLBACK_H

#include "connection_callback.h"
#include <libwebsockets.h>
#include <map>

namespace wsobj
{

class connection_callback_factory
{
public:
    virtual connection_callback* create(struct lws* wsi) = 0;
};

class protocol_dispatch_callback
{
public:
    connection_callback* create(struct lws* wsi)
    {
        const char* protocol = lws_get_protocol(wsi)->name;
        jgb_debug("{ protocol = %s }", protocol);
        auto it = factories_.find(protocol);
        if(it != factories_.end())
        {
            return it->second->create(wsi);
        }
        jgb_error("protocol not found.");
        return nullptr;
    }

    int install(const std::string& protocol, connection_callback_factory* factory)
    {
        if(factories_.find(protocol) != factories_.end())
        {
            return 1;
        }
        factories_[protocol] = factory;
        return 0;
    }

    static protocol_dispatch_callback* get_instance()
    {
        static protocol_dispatch_callback instance;
        return &instance;
    }

    std::map<std::string, connection_callback_factory*> factories_;
};

}

#endif // SERVER_CALLBACK_H
