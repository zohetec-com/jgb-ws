#include <jgb/core.h>
#include <jgb/helper.h>

#include "send_buffer_callback.h"

static int init(void*)
{
    ws::protocol_dispatch_callback::get_instance()->install(
        "sendbuffer", ws::send_buffer_callback_factory::get_instance());
    return 0;
}

static int tsk_loop(void* worker)
{
    jgb::worker* w = (jgb::worker*) worker;
    jgb::reader* rd = w->get_reader(0);
    jgb::frame frm;
    int r;
    r = rd->request_frame(&frm);
    if(!r)
    {
        //jgb_debug("{ len = %d }", frm.len );
        ws::send_buffer_callback_factory::get_instance()->request_to_send();
        rd->release();
    }
    return 0;
}

static loop_ptr_t loops[] = { tsk_loop, nullptr };

static jgb_loop_t loop
{
    .setup = nullptr,
    .loops = loops,
    .exit = nullptr
};

jgb_api_t ws_send_buffer
{
    .version = MAKE_API_VERSION(0, 1),
    .desc = "send buffer over websocket",
    .init = init,
    .release = nullptr,
    .create = nullptr,
    .destroy = nullptr,
    .commit = nullptr,
    .loop = &loop
};
