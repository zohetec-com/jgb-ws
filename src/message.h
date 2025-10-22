#ifndef MESSAGE_H
#define MESSAGE_H

#include <jgb/config.h>
#include <chrono>
#include <jgb/log.h>
#include <jgb/config_factory.h>

namespace ws
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
    message(void* in = nullptr, int len = 0, int64_t id=0L)
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
        c->create("id", id);
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

    request(const std::string& o, const std::string& m, int64_t id)
    : message()
    {
        c->create("object", o);
        c->create("method", m);
        c->set("id", id);
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

    int64_t id()
    {
        int64_t id = 0L;
        c->get("id", id);
        return id;
    }

    enum class method_e
    {
        UNKNOWN,
        CREATE,
        READ,
        UPDATE,
        DELETE
    };

    enum method_e get_method()
    {
        std::string m = method();
        if(m == "create")
        {
            return method_e::CREATE;
        }
        else if(m == "read")
        {
            return method_e::READ;
        }
        else if(m == "update")
        {
            return method_e::UPDATE;
        }
        else if(m == "delete")
        {
            return method_e::DELETE;
        }
        return method_e::UNKNOWN;
    }
};

class response: public message
{
public:

    response(int64_t id, int status = 0)
        : message(nullptr, 0, id)
    {
        c->create("status", status);
        //jgb_debug("new response { %p }", this);
    }

    response(void* in, int len)
        : message(in, len)
    {
    }

    void ok()
    {
        c->set("status", 200);
    }

    void not_implemented()
    {
        c->set("status", 501);
    }

    void status(int code)
    {
        c->set("status", code);
    }

    int status()
    {
        return c->int64("status");
    }
};

}

#endif // MESSAGE_H
