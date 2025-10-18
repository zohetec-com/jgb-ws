#ifndef SERVER_CALLBACK_H
#define SERVER_CALLBACK_H

#include "connection_callback.h"
#include <map>

class connection_callback_factory
{
public:
    virtual connection_callback* create(struct lws* wsi) = 0;
};

class server_callback: public connection_callback_factory
{
public:
    connection_callback* create(struct lws* wsi) override
    {
        const char* name = lws_get_protocol(wsi)->name;
        jgb_debug("{ protocol = %s }", name);
        auto it = factories_.find(name);
        if(it != factories_.end())
        {
            return it->second->create(wsi);
        }
        jgb_error("protocol not found.");
        return nullptr;
    }

    int install(const std::string& name, connection_callback_factory* factory)
    {
        if(factories_.find(name) != factories_.end())
        {
            return 1;
        }
        factories_[name] = factory;
        return 0;
    }

    static server_callback* get_instance()
    {
        static server_callback instance;
        return &instance;
    }

private:
    std::map<std::string, connection_callback_factory*> factories_;
};

#endif // SERVER_CALLBACK_H
