#ifndef CONNECTION_CALLBACK_H
#define CONNECTION_CALLBACK_H

#include <libwebsockets.h>
#include <string>
#include <inttypes.h>

class connection_callback
{
public:
    connection_callback(lws* wsi = nullptr);
    virtual ~connection_callback();

    virtual void on_connected() {}
    virtual void on_close() {}
    virtual void on_error() {}

    virtual void on_send() {}
    virtual void on_recv(void */*in*/, int /*len*/) {}

    void request_to_send();

    virtual void send(const char* buf, int len);
    virtual void send(const std::string& str);

    virtual void recv(void* in, int len);

    static const int send_buf_size_ = 1024*1024;
    static const int recv_buf_size_ = 1024*1024;

    uint8_t* send_buf_;
    uint8_t* recv_buf_;

    void* wsi_;
    int received_;

    bool print_sent_recv_;

private:
    uint8_t* internal_send_buf_;
    void append(void* in, int len);
};

#endif // CONNECTION_CALLBACK_H
