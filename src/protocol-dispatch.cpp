#include <jgb/core.h>
#include <jgb/helper.h>

#include "object_callback.h"

static int init(void*)
{
    ws::protocol_dispatch_callback::get_instance()->install(
        "ws-sync", ws::object_dispatch_callback::get_instance());
    return 0;
}

jgb_api_t protocol_dispatch
{
    .version = MAKE_API_VERSION(0, 1),
    .desc = "dispatch protocol",
    .init = init,
    .release = nullptr,
    .create = nullptr,
    .destroy = nullptr,
    .commit = nullptr,
    .loop = nullptr
};
