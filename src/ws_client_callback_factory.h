#ifndef WS_CLIENT_CALLBACK_FACTORY_H
#define WS_CLIENT_CALLBACK_FACTORY_H

#include "client_factory.h"
#include "ws-client.h"

class ws_client_callback_factory: public client_callback_factory
{
public:

    static ws_client_callback_factory* get_instance()
    {
        static ws_client_callback_factory instance;
        return &instance;
    }

    client_callback* create(jgb::config* conf) override
    {
        return new ws_client_callback(conf);
    }
};

#endif // WS_CLIENT_CALLBACK_FACTORY_H
