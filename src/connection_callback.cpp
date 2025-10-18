#include "connection_callback.h"
#include <jgb/log.h>
#include <libwebsockets.h>

connection_callback::connection_callback(lws *wsi)
    : wsi_(wsi),
    received_(0),
    print_sent_recv_(false)
{
    internal_send_buf_ = (uint8_t*) malloc(LWS_PRE + send_buf_size_);
    send_buf_ = internal_send_buf_ + LWS_PRE;
    recv_buf_ = (uint8_t*) malloc(recv_buf_size_);
    jgb_assert(internal_send_buf_);
    jgb_assert(recv_buf_);
}

connection_callback::~connection_callback()
{
    jgb_assert(internal_send_buf_);
    jgb_assert(recv_buf_);
    free(internal_send_buf_);
    free(recv_buf_);
}

void connection_callback::send(const std::string& str)
{
    send(str.c_str(), str.length());
}

void connection_callback::send(const char *buf, int len)
{
    jgb_assert(wsi_);
    struct lws* wsi = (struct lws*) wsi_;
    if(len > 0)
    {
        if(len < send_buf_size_)
        {
            int n;
            memcpy(send_buf_, buf, len);
            n = lws_write(wsi, send_buf_, len, LWS_WRITE_TEXT);
            if(n != len)
            {
                jgb_error("lws_write. { len = %d, n = %d }", len, n);
            }
        }
        else
        {
            jgb_error("发送长度超过限制。 { len = %d, send_buf_size_ = %d }", len, send_buf_size_);
        }
    }
    // 发送后自动请求再发送。
    lws_callback_on_writable(wsi);
}

void connection_callback::append(void* in, int len)
{
    if(len + received_ < recv_buf_size_)
    {
        memcpy(recv_buf_ + received_, in, len);
        received_ += len;
    }
    else
    {
        received_ += len;
        jgb_error("累计接收长度超过限制。 { received_ = %d, recv_buf_size_ = %d }",
                  received_, recv_buf_size_);
    }
}

void connection_callback::recv(void *in, int len)
{
    jgb_assert(wsi_);
    struct lws* wsi = (struct lws*) wsi_;
    bool start = lws_is_first_fragment(wsi);
    bool end = lws_is_final_fragment(wsi);

    if(!received_)
    {
        if(start && end)
        {
            on_recv(in, len);
        }
        else if(start)
        {
            append(in, len);
        }
        else
        {
            jgb_error("未接收到开始分片。{ len = %d }", len);
        }
    }
    else
    {
        if(start & end)
        {
            jgb_error("未接收到结束分片。{ received_ = %d }", received_);
            received_ = 0;
            on_recv(in, len);
        }
        else if(start)
        {
            jgb_error("未接收到结束分片。{ received_ = %d }", received_);
            received_ = 0;
            append(in, len);
        }
        else
        {
            append(in, len);
            if(end)
            {
                on_recv(recv_buf_, received_);
                received_ = 0;
            }
        }
    }
}
