#include <jgb/log.h>
#include <jgb/error.h>
#include <libwebsockets.h>
#include "sync_service_client.h"
#include <unistd.h>

enum conn_state
{
    disconnected = 0,
    connecting,
    connected
};

static int callback_minimal(struct lws* wsi, enum lws_callback_reasons reason,
                            void* user, void* in, size_t len);

static const struct lws_protocols protocols[] = {
    { "lws-minimal-client", callback_minimal, 0, 0, 0, NULL, 0 },
    LWS_PROTOCOL_LIST_TERM
};
static struct lws_context *context = nullptr;
static lws_retry_bo_t retry = {};
static const char* server = "127.0.0.1";
static int port = 8000;
static conn_state state = disconnected;
static bool reconnect = false;
static int64_t last = 0L;

static int callback_minimal(struct lws* wsi, enum lws_callback_reasons reason,
                            void*, void* in, size_t len)
{
    //sync_service_client* client = (sync_service_client*) user;
    //jgb_debug("{ wsi = %p, reason = %d, user = %p, in = %p, len = %lu }", wsi, reason, user, in, len);
    switch (reason) {
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        jgb_error("client error. { wsi = %p }", wsi);
        if(state == connecting)
        {
            reconnect = true;
        }
        else
        {
            jgb_error("{ state = %d }", state);
            jgb_assert(0);
        }
        break;

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        jgb_notice("client connected. { wsi = %p }", wsi);
        if(state == connecting)
        {
            state = connected;
        }
        else
        {
            jgb_error("{ state = %d }", state);
            jgb_assert(0);
        }
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
        sync_service_client::get_instance()->recv(in, len);
        break;

    case LWS_CALLBACK_CLIENT_CLOSED:
        jgb_notice("client closed. { wsi = %p }", wsi);
        if(state == connected)
        {
            state = connecting;
            reconnect = true;
        }
        else
        {
            jgb_error("{ state = %d }", state);
            jgb_assert(0);
        }
        break;

    default:
        break;
    }

    return 0;
}

static void do_connect(const char* host)
{
    struct lws_client_connect_info info = {};
    info.context = context;
    info.port = port;
    info.address = host;
    info.host = host;
    info.origin = host;
    info.path = "/";
    info.protocol = "lws-minimal-client";
    info.retry_and_idle_policy = &retry;
    struct lws* wsi = lws_client_connect_via_info(&info);
    jgb_assert(wsi);
    jgb_debug("new client. { wsi = %p }", wsi);
    sync_service_client::get_instance()->wsi_ = wsi;
}

int main(int argc, char* argv[])
{
    int c;

    while ((c = getopt (argc, argv, "p:s:v")) != -1)
    {
        switch (c)
        {
        case 's':
            server = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'v':
            sync_service_client::get_instance()->dump_recv_ = true;
            break;
        default:
            break;
        }
    }

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof info);
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    context = lws_create_context(&info);
    if (!context)
    {
        jgb_error("lws init failed\n");
        return JGB_ERR_FAIL;
    }

    reconnect = true;
    state = connecting;

    int n;
    do
    {
        if(reconnect)
        {
            int64_t now = get_clocktime_ms();
            if(now - last > 6000)
            {
                jgb_notice("reconnect");
                do_connect(server);
                reconnect = false;
                last = now;
            }
        }
        n = lws_service(context, 0);
    } while(n >= 0);

    return 0;
}
