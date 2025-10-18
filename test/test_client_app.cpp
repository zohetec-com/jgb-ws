#include <jgb/core.h>
#include <jgb/helper.h>
#include "test_client.h"

struct context_93f37fdfc071
{
    test_client* client;

    context_93f37fdfc071()
        : client(nullptr)
    {
    }

    ~context_93f37fdfc071()
    {
        delete client;
    }
};

static int tsk_init(void* worker)
{
    jgb::worker* w = (jgb::worker*) worker;
    context_93f37fdfc071* ctx = new context_93f37fdfc071;
    jgb::config* cfg = w->get_config();
    ctx->client = new test_client(cfg);
    w->set_user(ctx);
    jgb_assert(w);
    return 0;
}

static int tsk_loop(void* worker)
{
    jgb::worker* w = (jgb::worker*) worker;
    context_93f37fdfc071* ctx = (context_93f37fdfc071*) w->get_user();
    return ctx->client->process();
}

static void tsk_exit(void* worker)
{
    jgb::worker* w = (jgb::worker*) worker;
    context_93f37fdfc071* ctx = (context_93f37fdfc071*) w->get_user();
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

jgb_api_t sync_client
{
    .version = MAKE_API_VERSION(0, 1),
    .desc = "ws test client",
    .init = nullptr,
    .release = nullptr,
    .create = nullptr,
    .destroy = nullptr,
    .commit = nullptr,
    .loop = &loop
};
