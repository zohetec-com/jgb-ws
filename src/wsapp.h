#ifndef WSAPP_H_20250411
#define WSAPP_H_20250411

#include <jgb-ws/connection_callback.h>
#include <string>

typedef struct connect_request
{
    std::string protocol;
    std::string url;
    connection_callback* callback;
} connect_request_t;

int request_to_connect(connect_request_t& req);
void request_to_send(void* wsi);
void request_to_disconnect(void* wsi);
int get_peer_address(void* wsi, std::string& address, int port);
std::string get_hostname(const std::string& url);

#endif // WSAPP_H
