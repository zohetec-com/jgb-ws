#ifndef SEND_BUFFER_CALLBACK_H
#define SEND_BUFFER_CALLBACK_H

#include <jgb/config_factory.h>
#include <jgb/core.h>
#include <jgb/buffer.h>
#include <jgb/log.h>
#include <jgb/helper.h>
#include "connection_callback.h"
#include "protocol_dispatch_callback.h"
#include "wsapp.h"
#include <mutex>
#include <set>

namespace ws
{

class send_buffer_callback: public connection_callback
{
public:

    send_buffer_callback(void* wsi)
    : rd_(nullptr)
    {
        std::string id;
        int r;
        r = jgb::core::get_instance()->root_conf()->get("/ws_send_buffer/task/readers/buf_id", id);
        jgb_debug("{ r = %d, id = %s }", r, id.c_str());
        if(!r)
        {
            jgb::buffer* from_buf = jgb::buffer_manager::get_instance()->add_buffer(id);
            jgb_assert(from_buf);
            rd_ = from_buf->add_reader();
        }
        else
        {
            jgb_assert(0);
        }

        wsi_ = wsi;
        jgb_assert(wsi_);
        jgb_assert(rd_);

        frm_.buf = nullptr;
        frm_.len = 0;
        frm_.start_offset = 0;
        sent_ = 0;
        remain_ = 0;
    }

    ~send_buffer_callback()
    {
        jgb::buffer* buf = rd_->buf_;
        buf->remove_reader(rd_);
        jgb::buffer_manager::get_instance()->remove_buffer(buf);
    }

    void on_send() override
    {
        int r;
        int to_send = 0;
        int is_start = true;
        int is_end = true;

        if(!remain_)
        {
            r = rd_->request_frame(&frm_, 100);
            if(!r)
            {
                jgb_assert(frm_.buf);
                jgb_assert(frm_.len > 0);
                jgb_assert(frm_.start_offset == 0);

                //jgb_debug("len = %d", frm_.len);

                remain_ = frm_.len;
                sent_ = 0;
            }
            else
            {
                return;
            }
        }
        else
        {
            jgb_assert(frm_.buf);
            jgb_assert(frm_.len > 0);

            is_start = false;
        }

        jgb_assert(remain_ > 0);

        if(remain_ > connection_callback::send_buf_size_)
        {
            is_end = false;
            to_send = connection_callback::send_buf_size_;
        }
        else
        {
            to_send = remain_;
        }

        jgb_assert(to_send > 0);

        //jgb_debug("{ to send = %d }", to_send );
        send((const char*) frm_.buf + sent_, to_send, lws_write_ws_flags(LWS_WRITE_BINARY, is_start, is_end));
        sent_ += to_send;
        remain_ -= to_send;

        if(!remain_)
        {
            jgb_assert(sent_ == frm_.len);
            jgb_assert(is_end);
            rd_->release();
        }
    }

    // 请求发送。
    void report()
    {
        //jgb_function();
        jgb_assert(wsi_);
        ::request_to_send(wsi_);
    }

public:
    jgb::reader* rd_;  // 从缓冲区读取数据，以 websocket 发送。
    jgb::frame frm_;
    int sent_;
    int remain_;
};


class send_buffer_callback_factory: public connection_callback_factory
{
public:

    static send_buffer_callback_factory* get_instance()
    {
        static send_buffer_callback_factory instance;
        return &instance;
    }

    connection_callback* create(struct lws* wsi) override
    {
        send_buffer_callback* ctx = new send_buffer_callback(wsi);
        ctx->factory_ = this;
        std::lock_guard<std::mutex> lock(mutex_);
        ctx_set_.insert(ctx);
        return ctx;
    }

    void remove(connection_callback* cb) override
    {
        jgb_assert(cb);
        std::lock_guard<std::mutex> lock(mutex_);
        auto i = ctx_set_.find(cb);
        if(i != ctx_set_.end())
        {
            ctx_set_.erase(i);
            connection_callback_factory::remove(cb);
        }
        else
        {
            jgb_assert(0);
        }
    }

    void request_to_send() override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for(auto ctx: ctx_set_)
        {
            send_buffer_callback* cb = (send_buffer_callback*) ctx;
            cb->report();
        }
    }

private:
    std::mutex mutex_;
    std::set<void*> ctx_set_;
};

}

#endif // SEND_BUFFER_CALLBACK_H
