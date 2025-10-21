#include "object_callback.h"

namespace wsobj
{

void connection_context::on_recv(void *in, int len)
{
    jgb_info("%.*s", len, in);
    object_dispatch_callback::get_instance()->process(*this, in, len);
}

}
