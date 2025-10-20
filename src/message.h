#ifndef MESSAGE_H
#define MESSAGE_H

#include <jgb/config.h>
#include <chrono>
#include <jgb/log.h>
#include <jgb/config_factory.h>

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

class message
{
public:
    message(void* in = nullptr, int len = 0)
        : c(nullptr)
    {
        if(in && len)
        {
            c = jgb::config_factory::create((char*) in, len);
        }
        if(!c)
        {
            c = new jgb::config;
        }
        c->create("time", get_timestamp_us());
        c->create("id", 0);
    }

    virtual ~message()
    {
        delete c;
    }

    std::string to_string()
    {
        return c->to_string();
    }

    // TODO: rename c
    jgb::config* c;
};

class request: public message
{
public:
    request(void* in = nullptr, int len = 0)
        : message(in, len)
    {
    }

    request(const std::string& o, const std::string& m)
    : message()
    {
        c->create("object", o);
        c->create("method", m);
    }

    ~request()
    {
    }

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

class response: public message
{
public:

    response(int status = 0)
        : message()
    {
        c->create("status", status);
        //jgb_debug("new response { %p }", this);
    }

    void ok()
    {
        c->set("status", 200);
    }
};

}

#endif // MESSAGE_H
