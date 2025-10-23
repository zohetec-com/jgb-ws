#include <jgb/core.h>
#include <jgb/helper.h>
#include "ws_client_callback_factory.h"

struct context_24e8546255cf
{
    client_callback* client;

    context_24e8546255cf()
        : client(nullptr)
    {
    }

    ~context_24e8546255cf()
    {
        delete client;
    }
};

static int init(void*)
{
    client_factory::get_instance()->install("ws-client", ws_client_callback_factory::get_instance());
    return 0;
}

static int tsk_init(void* worker)
{
    jgb::worker* w = (jgb::worker*) worker;
    context_24e8546255cf* ctx = new context_24e8546255cf;
    jgb::config* cfg = w->get_config();
    std::string type = "ws-client";
    cfg->get("type", type);
    ctx->client = client_factory::get_instance()->create(type, cfg);
    w->set_user(ctx);
    jgb_assert(w);
    return 0;
}

static int tsk_loop(void* worker)
{
    jgb::worker* w = (jgb::worker*) worker;
    context_24e8546255cf* ctx = (context_24e8546255cf*) w->get_user();
    return ctx->client->process();
}

static void tsk_exit(void* worker)
{
    jgb::worker* w = (jgb::worker*) worker;
    context_24e8546255cf* ctx = (context_24e8546255cf*) w->get_user();
    ctx->client->disconect();
    while(ctx->client->running())
    {
        jgb_notice("wait for close. { url = %s, state = %d }",
                   ctx->client->url_.c_str(), (int) ctx->client->state_);
        jgb::sleep(200);
    }
    jgb_assert(w);
    delete ctx;
}

static loop_ptr_t loops[] = { tsk_loop, nullptr };

static jgb_loop_t loop
{
    .setup = tsk_init,
    .loops = loops,
    .exit = tsk_exit
};

jgb_api_t ws_client
{
    .version = MAKE_API_VERSION(0, 1),
    .desc = "ws client",
    .init = init,
    .release = nullptr,
    .create = nullptr,
    .destroy = nullptr,
    .commit = nullptr,
    .loop = &loop
};
