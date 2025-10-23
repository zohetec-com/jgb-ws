#ifndef CLIENT_FACTORY_H
#define CLIENT_FACTORY_H

#include "client_callback.h"
#include <map>

class client_callback_factory
{
public:
    virtual client_callback* create(jgb::config* conf) = 0;
};

class client_factory
{
public:

    client_callback* create(const std::string& type, jgb::config* conf)
    {
        auto it = factories_.find(type);
        if(it != factories_.end())
        {
            return it->second->create(conf);
        }
        jgb_error("client type not found. { type = %s }", type.c_str());
        return nullptr;
    }

    static client_factory* get_instance()
    {
        static client_factory instance;
        return &instance;
    }

    int install(const std::string& type, client_callback_factory* factory)
    {
        if(factories_.find(type) != factories_.end())
        {
            return 1;
        }
        factories_[type] = factory;
        return 0;
    }

    std::map<std::string, client_callback_factory*> factories_;
};

#endif // CLIENT_FACTORY_H
