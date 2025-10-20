#ifndef CLIENT_CALLBACK_H
#define CLIENT_CALLBACK_H

#include <jgb/config.h>
#include <jgb/log.h>
#include <jgb/config_factory.h>
#include <string>
#include "connection_callback.h"
#include "wsapp.h"

class client_callback: public connection_callback
{
public:
    enum class state
    {
        idle = 0,
        connecting,
        connected,
        aborting,
        closed
    };

    client_callback(jgb::config* conf)
        : state_(state::idle),
          conf_(conf),
          reconnect_(true)
    {
        conf_->create("state", (int) state_);
        conf_->get("protocol", protocol_);
        conf_->get("url", url_);
        jgb_info("{ url = %s }", url_.c_str());
    }

    void to_state(state s)
    {
        state_ = s;
        conf_->set("state", (int) s);
    }

    virtual void on_connected() override
    {
        jgb_assert(state_ == state::connecting);
        jgb_assert(wsi_);
        to_state(state::connected);
        jgb_info("ws connected");
    }

    virtual void on_close() override
    {
        // 连接关闭。
        to_state(state::closed);
    }

    virtual void on_error() override
    {
        // 连接错误。
        to_state(state::aborting);
        jgb_warning("connection error. { url = %s }", url_.c_str());
    }

    virtual void on_session()
    {
        jgb::sleep(1000);
    }

    virtual int process()
    {
        if(state_ == state::idle)
        {
            if(url_.empty())
            {
                jgb_warning("no url.");
                return JGB_ERR_END;
            }

            jgb_info("connecting ... { url = %s }", url_.c_str());
            to_state(state::connecting);

            connect_request_t req = { protocol_, url_, this };
            int r;
            r = request_to_connect(req);
            if(!r)
            {
            }
            else
            {
                to_state(state::idle);

                jgb_debug("waiting ...");
                jgb::sleep(100);
            }
        }
        else if(state_ == state::connecting)
        {
            // 等待连接成功。
            jgb_debug("waiting for connected. { url = %s }", url_.c_str());
            jgb::sleep(200);
        }
        else if(state_ == state::connected)
        {
            on_session();
        }
        else if(state_ == state::aborting)
        {
            jgb_debug("aborted. { url = %s }", url_.c_str());
            to_state(state::closed);
        }
        else if(state_ == state::closed)
        {
            jgb_debug("closed. { url = %s }", url_.c_str());
            jgb::sleep(200);
            if(reconnect_)
            {
                jgb_debug("reconnect. { url = %s }", url_.c_str());
                // reset.
                to_state(state::idle);
            }
            else
            {
                jgb_debug("no reconnect. { url = %s }", url_.c_str());
                return JGB_ERR_END;
            }
        }
        else
        {
            jgb_assert(0);
        }
        return 0;
    }

    void disconect()
    {
        reconnect_ = false;
        if(state_ != state::closed)
        {
            request_to_disconnect(wsi_);
        }
    }

    bool running()
    {
        return !(state_ == state::closed || state_ == state::idle);
    }

    state state_;
    jgb::config* conf_;
    std::string protocol_;
    std::string url_;
    bool reconnect_;
};

#endif // CLIENT_CALLBACK_H
