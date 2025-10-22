#include <jgb/core.h>
#include <jgb/helper.h>

#include "protocol_dispatch_callback.h"
#include "object_callback.h"
#include "hello_object_callback.h"

static int init(void*)
{
    ws::protocol_dispatch_callback::get_instance()->install(
        "ws-sync", ws::object_dispatch_callback::get_instance());
    ws::object_dispatch_callback::get_instance()->install(
        "hello", hello_object_callback::get_instance());
    return 0;
}

jgb_api_t test_server
{
    .version = MAKE_API_VERSION(0, 1),
    .desc = "test ws object",
    .init = init,
    .release = nullptr,
    .create = nullptr,
    .destroy = nullptr,
    .commit = nullptr,
    .loop = nullptr
};
