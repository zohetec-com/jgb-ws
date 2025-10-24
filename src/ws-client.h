#ifndef WS_CLIENT_H_20251024
#define WS_CLIENT_H_20251024

#include <jgb/buffer.h>
#include "client_callback.h"

class ws_client_callback: public client_callback
{
public:

    virtual void on_send() override
    {
        if(rd_)
        {
            jgb::frame frm;
            int r;
            r = rd_->request_frame(&frm, 100);
            if(!r)
            {
                //jgb_debug("send. { %.*s }", frm.len, frm.buf);
                connection_callback::send((const char*) frm.buf, frm.len);
                rd_->release();
            }
        }
    }

    virtual void on_recv(void *in, int len) override
    {
        if(wr_)
        {
            wr_->put((uint8_t*) in, len);
        }
    }

    ws_client_callback(jgb::config* conf)
        : client_callback(conf)
    {
        buf_[0] = nullptr;
        buf_[1] = nullptr;
        rd_ = nullptr;
        wr_ = nullptr;

        int r;
        std::string buf_id;
        r = conf->get("task/readers/buf_id", buf_id);
        if(!r)
        {
            buf_[0] = jgb::buffer_manager::get_instance()->add_buffer(buf_id);
            rd_ = buf_[0]->add_reader(true);
        }
        r = conf->get("task/writers/buf_id", buf_id);
        if(!r)
        {
            buf_[1] = jgb::buffer_manager::get_instance()->add_buffer(buf_id);
            int size = 0;
            r = conf->get("task/writers/buf_size", size);
            if(!r)
            {
                buf_[1]->resize(size);
            }
            wr_ = buf_[1]->add_writer();
        }
    }

    ~ws_client_callback()
    {
        if(buf_[0])
        {
            buf_[0]->remove_reader(rd_);
            jgb::buffer_manager::get_instance()->remove_buffer(buf_[0]);
        }
        if(buf_[1])
        {
            buf_[1]->remove_writer(wr_);
            jgb::buffer_manager::get_instance()->remove_buffer(buf_[1]);
        }
    }

private:
    jgb::buffer* buf_[2];
    jgb::reader* rd_;
    jgb::writer* wr_;
};

#endif // WS_CLIENT_H_20251024
