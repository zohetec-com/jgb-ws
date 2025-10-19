#include <jgb/core.h>
#include <jgb/helper.h>

#include "sqlite_callback.h"

static int init(void*)
{
    wsobj::object_callback_dispatcher::get_instance()->install("xxx", wsobj::sqlite_callback::get_instance());
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
