#ifndef CONNECTION_CALLBACK_H
#define CONNECTION_CALLBACK_H

#include <libwebsockets.h>
#include <string>
#include <inttypes.h>

namespace ws
{

class connection_callback_factory;
class connection_callback
{
public:
    connection_callback(lws* wsi = nullptr, int recv_buf_size = 0);
    virtual ~connection_callback();

    virtual void on_connected() {}
    virtual void on_close() {}
    virtual void on_error() {}

    virtual void on_send() {}
    virtual void on_recv(void */*in*/, int /*len*/) {}

    void request_to_send();

    virtual void send(const char* buf, int len, uint32_t protocol = LWS_WRITE_TEXT);
    virtual void send(const std::string& str);

    virtual void recv(void* in, int len);
    int resize_recv_buffer(int new_size);

    static const int send_buf_size_ = 16*1024;

    uint8_t* send_buf_;
    uint8_t* recv_buf_;

    void* wsi_;
    int received_;

    bool print_sent_recv_;

    // for server end connection
    connection_callback_factory* factory_;

    int recv_buf_size_;

private:
    uint8_t* internal_send_buf_;
    void append(void* in, int len);
};

}
#endif // CONNECTION_CALLBACK_H
