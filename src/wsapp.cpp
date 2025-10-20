#include "wsapp.h"
#include <jgb/core.h>
#include <jgb/helper.h>
#include <libwebsockets.h>
#include <inttypes.h>
#include <mutex>
#include <set>
#include <curl/curl.h>
#include "protocol_dispatch_callback.h"

static int callback_minimal(struct lws* wsi, enum lws_callback_reasons reason,
                            void* user, void* in, size_t len);

struct connect_request_cmp
{
    bool operator()(const connect_request_t& lhs, const connect_request_t& rhs) const
    {
        return lhs.url < rhs.url;
    }
};

static struct lws_context *context = nullptr;
static std::set<connect_request_t,connect_request_cmp> to_connect_set;
static std::set<void*> to_send_set;
static std::set<void*> to_disconnect_set;
static std::set<void*> live_set;
static std::mutex mutex;
static lws_retry_bo_t retry = {};
static bool print_sent_recv = false;
static std::string iface;
struct lws_protocols *protocols = nullptr;

int get_peer_address(void* wsi, std::string& address, int)
{
    char name[64];
    name[0] = '\0';
    lws_get_peer_simple((struct lws*) wsi, name, 64);
    address = std::string(name);
    return 0;
}

int request_to_connect(connect_request_t& req)
{
    if(context)
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = to_connect_set.insert(req);
        if(it.second)
        {
            lws_cancel_service(context);
        }
        else
        {
            jgb_warning("connect request already exist. { url = \"%s\" }", req.url.c_str());
        }
        return 0;
    }
    else
    {
        jgb_error("invalid context");
        return JGB_ERR_FAIL;
    }
}

void request_to_send(void* wsi)
{
    if(context && wsi)
    {
        jgb_assert(context);
        std::lock_guard<std::mutex> lock(mutex);
        to_send_set.insert(wsi);
        lws_cancel_service(context);
    }
    else
    {
        jgb_error("invalid context");
    }
}

void request_to_disconnect(void* wsi)
{
    if(context && wsi)
    {
        jgb_assert(context);
        std::lock_guard<std::mutex> lock(mutex);
        to_disconnect_set.insert(wsi);
        lws_cancel_service(context);
        jgb_notice("request_to_disconnect. { wsi = %p, size = %lu }", wsi, to_disconnect_set.size());
    }
    else
    {
        jgb_error("invalid context");
    }
}

static void add_live(struct lws* wsi)
{
    live_set.insert(wsi);
}

static void remove_live(struct lws* wsi)
{
    auto i = live_set.find(wsi);
    if(i != live_set.end())
    {
        live_set.erase(i);
    }
}

static int callback_minimal(struct lws* wsi, enum lws_callback_reasons reason,
                             void* user, void* in, size_t len)
{
    connection_callback* callback;
    //jgb_debug("{ wsi = %p, reason = %d, user = %p }", wsi, reason, user);
    switch (reason) {
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            if(user)
            {
                callback = (connection_callback*) user;
                callback->on_error();
                remove_live(wsi);
            }
            else
            {
                jgb_assert(0);
            }
            jgb_error("client error. { wsi = %p }", wsi);
            break;

        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            if(user)
            {
                add_live(wsi);
                callback = (connection_callback*) user;
                callback->wsi_ = wsi;
                callback->print_sent_recv_ = print_sent_recv;
                callback->on_connected();
                // 连接成功后自动请求发送。
                lws_callback_on_writable(wsi);
            }
            else
            {
                jgb_assert(0);
            }
            jgb_notice("client connected. { wsi = %p }", wsi);
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            if(user)
            {
                // 处理主动断开与远程服务器连接的请求。
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    auto i = to_disconnect_set.find(wsi);
                    if(i != to_disconnect_set.end())
                    {
                        to_disconnect_set.erase(i);
                        jgb_warning("disconnect. { wsi = %p }", wsi);
                        // 断开连接。
                        return -1;
                    }
                }

                callback = (connection_callback*) user;
                callback->on_send();
            }
            else
            {
                jgb_assert(0);
            }
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            if(user)
            {
                callback = (connection_callback*) user;
                callback->recv(in, len);
            }
            else
            {
                jgb_assert(0);
            }
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            jgb_notice("client closed. { wsi = %p }", wsi);
            if(user)
            {
                callback = (connection_callback*) user;
                callback->on_close();
                remove_live(wsi);
            }
            else
            {
                jgb_assert(0);
            }
            break;

        case LWS_CALLBACK_ESTABLISHED:
            {
                add_live(wsi);
            connection_callback* cb = wsobj::protocol_dispatch_callback::get_instance()->create(wsi);
                if(cb)
                {
                    lws_set_wsi_user(wsi, cb);
                    cb->on_connected();
                }
                else
                {
                    jgb_assert(0);
                }
            }
            break;

        case LWS_CALLBACK_CLOSED:
            {
                connection_callback* cb = (connection_callback*) lws_wsi_user(wsi);
                delete cb;
                remove_live(wsi);
            }
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            {
                connection_callback* cb = (connection_callback*) lws_wsi_user(wsi);
                cb->on_send();
            }
            break;

        case LWS_CALLBACK_RECEIVE:
            {
                connection_callback* cb = (connection_callback*) lws_wsi_user(wsi);
                cb->recv(in, len);
            }
            break;

        default:
            break;
    }

    return 0;
}

static int tsk_init(void* worker)
{
    jgb::worker* w = (jgb::worker*) worker;
    jgb_assert(w);
    jgb_assert(w->task_);
    jgb_assert(w->task_->instance_);
    jgb_assert(!w->task_->instance_->user_);

    jgb_assert(!protocols);

    jgb_debug("factories %d", wsobj::protocol_dispatch_callback::get_instance()->factories_.size());

    uint i = 0;
    if(wsobj::protocol_dispatch_callback::get_instance()->factories_.size() > 0)
    {
        protocols = new lws_protocols[wsobj::protocol_dispatch_callback::get_instance()->factories_.size() + 1];
        for(auto it: wsobj::protocol_dispatch_callback::get_instance()->factories_)
        {
            protocols[i].name = strdup(it.first.c_str());
            protocols[i].callback = callback_minimal;
            protocols[i].per_session_data_size = 0;
            protocols[i].rx_buffer_size = 0;
            protocols[i].id = 0;
            protocols[i].user = nullptr;
            protocols[i].tx_packet_size = 0;

            jgb_debug("{ i = %d, protocol = %s }", i, protocols[i].name);

            i++;
        }
        //jgb_assert(0);
    }
    else
    {
        protocols = new lws_protocols[2];
        protocols[i].name = "ws-object";
        protocols[i].callback = callback_minimal;
        protocols[i].per_session_data_size = 0;
        protocols[i].rx_buffer_size = 0;
        protocols[i].id = 0;
        protocols[i].user = nullptr;
        protocols[i].tx_packet_size = 0;
        i++;
    }

    protocols[i].name = nullptr;
    protocols[i].callback = nullptr;
    protocols[i].per_session_data_size = 0;
    protocols[i].rx_buffer_size = 0;
    protocols[i].id = 0;
    protocols[i].user = nullptr;
    protocols[i].tx_packet_size = 0;

    w->get_config()->get("print_sent_recv", print_sent_recv);
    w->get_config()->get("iface", iface);

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof info);
    if(!iface.empty())
    {
        info.iface = iface.c_str();
    }
    info.port = CONTEXT_PORT_NO_LISTEN;
    w->get_config()->get("port", info.port);
    jgb_info("{ iface = %s, port = %d }",
             info.iface ? info.iface : "(null)", info.port);
    info.protocols = protocols;
    context = lws_create_context(&info);
    if (!context)
    {
        jgb_error("lws init failed\n");
        return JGB_ERR_FAIL;
    }

    return 0;
}

static void to_ack(std::set<void*>& ack_set)
{
    for(auto i:ack_set)
    {
        auto j = live_set.find(i);
        if(j != live_set.end())
        {
            lws_callback_on_writable((struct lws*)i);
        }
        else
        {
            jgb_error("invalid connection.{ wsi = %p }", i);
        }
    }
}

static void to_connect()
{
    for(auto i:to_connect_set)
    {
        CURLU *h = curl_url();
        if(h)
        {
            char *host = nullptr;
            char *port = nullptr;
            char *path = nullptr;
            jgb_info("{ url = \"%s\" }", i.url.c_str());
            int rc = curl_url_set(h, CURLUPART_URL, i.url.c_str(), CURLU_NON_SUPPORT_SCHEME);
            if(!rc)
            {
                rc = curl_url_get(h, CURLUPART_HOST, &host, 0);
                if(!rc)
                {
                    rc = curl_url_get(h, CURLUPART_PORT, &port, 0);
                    if(!rc)
                    {
                        int iport;
                        rc = jgb::stoi(port, iport);
                        if(!rc)
                        {
                            rc = curl_url_get(h, CURLUPART_PATH, &path, 0);
                            if(!rc)
                            {
#if 0
                                char* q;
                                rc = curl_url_get(h, CURLUPART_QUERY, &q, 0);
                                if(rc)
                                {
                                    //jgb_error("curl_url_get query failed");
                                    q = (char*) "";
                                }
                                std::string path_q = std::string(path) + std::string(q);
                                jgb_info("{ host = \"%s\", port = \"%s\", path = \"%s\", query = \"%s\", path_q = \"%s\" }",
                                         host, port, path, q, path_q.c_str());
#endif
                                jgb_info("{ protocol = \"%s\", host = \"%s\", port = \"%s\", path = \"%s\" }",
                                         i.protocol.c_str(),
                                         host, port, path);
                                struct lws_client_connect_info info = {};
                                info.context = context;
                                info.port = iport;
                                info.address = host;
                                info.host = host;
                                info.origin = host;
                                info.path = path;
                                info.protocol = i.protocol.c_str();
                                info.userdata = (void*)i.callback;
                                info.retry_and_idle_policy = &retry;
                                struct lws* wsi = lws_client_connect_via_info(&info);
                                jgb_assert(wsi);
                                jgb_debug("new client. { wsi = %p }", wsi);
                            }
                            else
                            {
                                jgb_error("curl_url_get path failed. { rc = %d }", rc);
                            }
                        }
                        else
                        {
                            jgb_error("invalid port. { port = \"%s\" }", port);
                        }
                    }
                    else
                    {
                        jgb_error("curl_url_get port failed. { rc = %d }", rc);
                    }
                }
                else
                {
                    jgb_error("curl_url_get host failed. { rc = %d }", rc);
                }
            }
            else
            {
                jgb_error("curl_url_set failed. { rc = %d }", rc);
            }
            if(host)
            {
                curl_free(host);
            }
            if(port)
            {
                curl_free(port);
            }
            if(path)
            {
                curl_free(path);
            }
            curl_url_cleanup(h);
        }
    }
    to_connect_set.clear();
}

static void prepare()
{
    std::lock_guard<std::mutex> lock(mutex);
    to_connect();
    to_ack(to_disconnect_set);
    to_ack(to_send_set);
    to_send_set.clear();
}

static int tsk_loop(void*)
{
    prepare();

    int n = lws_service(context, 0);
    if(n >= 0)
    {
        return 0;
    }
    else
    {
        jgb_fatal("lws_service. { n = %d }", n);
        return JGB_ERR_FAIL;
    }
}

// TODO：ip_cam 工作线程必须都已经结束。
static void tsk_exit(void*)
{
    jgb_debug("wsapp exit. { to_connect = %ld, to_send = %ld, to_disconnect = %ld }",
              to_connect_set.size(), to_send_set.size(), to_disconnect_set.size());
    lws_context_destroy(context);
    to_connect_set.clear();
    to_send_set.clear();
    to_disconnect_set.clear();
    live_set.clear();
    for(uint i=0; i<wsobj::protocol_dispatch_callback::get_instance()->factories_.size(); i++)
    {
        free((void*)protocols[i].name);
    }
    delete[] protocols;
}

static loop_ptr_t loops[] = { tsk_loop, nullptr };

static jgb_loop_t loop
{
    .setup = tsk_init,
    .loops = loops,
    .exit = tsk_exit
};

static int init(void*)
{
    retry.secs_since_valid_ping = 6;
    retry.secs_since_valid_hangup = 10;

    //lws_set_log_level(LLL_ERR|LLL_WARN|LLL_NOTICE|LLL_INFO|LLL_DEBUG, nullptr);

    return 0;
}

static void release(void*)
{
}

jgb_api_t wsapp
{
    .version = MAKE_API_VERSION(0, 1),
    .desc = "websocket app module",
    .init = init,
    .release = release,
    .create = nullptr,
    .destroy = nullptr,
    .commit = nullptr,
    .loop = &loop
};
