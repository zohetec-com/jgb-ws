#include <jgb/core.h>
#include <jgb/helper.h>

#include "sqlite_callback.h"

struct context_f0736a47718c
{
    wsobj::sqlite_callback* cb_;

    context_f0736a47718c()
        : cb_(nullptr)
    {
    }

    ~context_f0736a47718c()
    {
        delete cb_;
    }
};

static int create(void* conf)
{
    jgb::config* c = (jgb::config*) conf;
    jgb::instance* inst = jgb::instance::get_instance(c);
    context_f0736a47718c* ctx = new context_f0736a47718c();
    inst->set_user(ctx);

    std::string filename;
    int r;

    c->get("filename", filename);
    if(!filename.empty())
    {
        ctx->cb_ = new wsobj::sqlite_callback(filename);
        jgb::value* val;
        r = c->get("tables", &val);
        if(!r && val->type_ == jgb::value::data_type::string)
        {
            for(int i=0; i<val->len_; i++)
            {
                jgb_debug("{ i = %d, table = %s }", i, val->str_[i]);
                wsobj::object_dispatch_callback::get_instance()->install(
                    val->str_[i], ctx->cb_);
            }
        }
    }

    return 0;
}

static void destroy(void* conf)
{
    jgb::config* c = (jgb::config*) conf;
    jgb::instance* inst = jgb::instance::get_instance(c);
    context_f0736a47718c* ctx = (context_f0736a47718c*) inst->get_user();
    delete ctx;
}

jgb_api_t sqlite_app
{
    .version = MAKE_API_VERSION(0, 1),
    .desc = "sqlite app",
    .init = nullptr,
    .release = nullptr,
    .create = create,
    .destroy = destroy,
    .commit = nullptr,
    .loop = nullptr
};
