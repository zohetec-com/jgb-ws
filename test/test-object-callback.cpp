#include <jgb/core.h>
#include <jgb/helper.h>

#include "object_callback.h"

static int init(void*)
{
    wsobj::protocol_dispatch_callback::get_instance()->install(
        "ws-object", wsobj::object_dispatch_callback::get_instance());
    return 0;
}

jgb_api_t sqlite_app
{
    .version = MAKE_API_VERSION(0, 1),
    .desc = "sqlite object app",
    .init = init,
    .release = nullptr,
    .create = nullptr,
    .destroy = nullptr,
    .commit = nullptr,
    .loop = nullptr
};
