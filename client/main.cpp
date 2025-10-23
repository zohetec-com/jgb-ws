#include <csignal>
#include <jgb/config.h>
#include <jgb/log.h>
#include <jgb/core.h>
#include <jgb/helper.h>
#include <getopt.h>
#include "jansson.h"
#include <stack>
#include "message.h"
#include "ws-client.h"
#include "ws_client_callback_factory.h"

extern jgb_api_t wsapp;
extern jgb_api_t ws_client;
extern int jgb_print_level;

static bool exit_flag = false;

// 处理 SIGINT 信号
static void handler(int signum)
{
    jgb_notice("signal catched. { signum = %d }", signum);
    if(signum == SIGINT)
    {
        exit_flag = true;

        static int count = 0;
        ++ count;
        if(count > 10)
        {
            exit(1);
        }
    }
}

static int run(void)
{
    for(auto it = jgb::core::get_instance()->app_.begin(); it != jgb::core::get_instance()->app_.end(); ++it)
    {
        for(auto it2 = (*it)->instances_.begin(); it2 != (*it)->instances_.end(); ++it2)
        {
            (*it2)->start();
        }
    }
    return 0;
}

static void stop(void)
{
    for(auto it = jgb::core::get_instance()->app_.rbegin(); it != jgb::core::get_instance()->app_.rend(); ++it)
    {
        for(auto it2 = (*it)->instances_.rbegin(); it2 != (*it)->instances_.rend(); ++it2)
        {
            (*it2)->stop();
        }
    }
}

int main(int argc, char *argv[])
{
    // 注册 SIGINT 信号处理函数
    struct sigaction act = {};

    // https://man7.org/linux/man-pages/man7/signal.7.html
    act.sa_handler = &handler;
    if (sigaction(SIGINT, &act, NULL) == -1)
    {
        perror("sigaction");
    }
    if (sigaction(SIGUSR1, &act, NULL) == -1)
    {
        perror("sigaction");
    }

    const char* protocol = "ws-sync";
    const char* server = "127.0.0.1";
    const char* port = "8000";

    static struct option long_options[] = {
        {"protocol", required_argument, 0, 0},
        {0, 0, 0, 0} // Sentinel to mark the end of the array
    };

    int long_index = 0; // To store the index of the long option found
    int opt;
    //jgb::config* conf = new jgb::config;
    std::stack<json_t*> parent;
    json_t* jreq = json_object();
    json_t* jcur = jreq;
    const char* key = nullptr;
    jgb_assert(jreq);
    double interval = 1.0;
    bool loop = false;

    while ((opt = getopt_long(argc, argv, "A::dh:i:I:K:lm:O::o:p:t:T:Uv:", long_options, &long_index)) != -1) {
        switch (opt) {
        case 0: // This case is for long options that don't have a short option equivalent
            jgb_debug("{ index = %d, opt = %s }", long_index, long_options[long_index].name);
            if(long_index == 0) // protocol
            {
                protocol = optarg;
            }
            break;
        case 'h':
            server = optarg;
            break;
        case 'l':
            loop = true;
            break;
        case 'p':
            port = optarg;
            break;
        case 't':
            jgb::stod(optarg, interval);
            break;
        case 'v':
            jgb::stoi(optarg, jgb_print_level);
            break;

        case 'i':
            if(json_is_object(jcur))
            {
                int r;
                int64_t iv = 0L;
                r = jgb::stoll(optarg, iv);
                if(!r)
                {
                    json_t* j_iv = json_integer(iv);
                    json_object_set_new(jcur, "id", j_iv);
                }
                else
                {
                    json_t* j_tv = json_string(optarg);
                    json_object_set_new(jcur, "id", j_tv);
                }
            }
            else
            {
                jgb_warning("invalid");
            }
            break;

        case 'o':
            if(json_is_object(jcur))
            {
                json_t* j_tv = json_string(optarg);
                json_object_set_new(jcur, "object", j_tv);
            }
            break;

        case 'm':
            if(json_is_object(jcur))
            {
                json_t* j_tv = json_string(optarg);
                json_object_set_new(jcur, "method", j_tv);
            }
            break;

        case 'd':
            if(json_is_object(jcur))
            {
                json_t* j_a = json_array();
                json_object_set_new(jcur, "data", j_a);
                parent.push(jcur);
                jcur = j_a;
            }
            break;

        case 'A':
            if(json_is_object(jcur))
            {
                if(optarg)
                {
                    json_t* j_a = json_array();
                    json_object_set_new(jcur, optarg, j_a);
                    parent.push(jcur);
                    jcur = j_a;
                    key = nullptr;
                }
                else if(key)
                {
                    json_t* j_a = json_array();
                    json_object_set_new(jcur, key, j_a);
                    parent.push(jcur);
                    jcur = j_a;
                    key = nullptr;
                }
                else
                {
                    jgb_warning("no key");
                }
            }
            else
            {
                jgb_fail("not support");
            }
            break;

        case 'K':
            key = optarg;
            break;

        case 'I':
            if(json_is_object(jcur))
            {
                if(key)
                {
                    int64_t iv = 0L;
                    int r = jgb::stoll(optarg, iv);
                    if(!r)
                    {
                        json_t* j_iv = json_integer(iv);
                        json_object_set_new(jcur, key, j_iv);
                    }
                    key = nullptr;
                }
                else
                {
                    jgb_warning("no key");
                }
            }
            else if(json_is_array(jcur))
            {
                int64_t iv = 0L;
                int r = jgb::stoll(optarg, iv);
                if(!r)
                {
                    json_t* j_iv = json_integer(iv);
                    json_array_append_new(jcur, j_iv);
                }
            }
            else
            {
                jgb_assert(0);
            }
            break;

        case 'O':
            if(optarg)
            {
                json_t* o = json_object();
                json_object_set_new(jcur, optarg, o);
                parent.push(jcur);
                jcur = o;
            }
            else if(json_is_array(jcur))
            {
                json_t* o = json_object();
                json_array_append_new(jcur, o);
                parent.push(jcur);
                jcur = o;
            }
            else
            {
                jgb_warning("invalid");
            }
            break;

        case 'T':
            if(json_is_object(jcur))
            {
                if(key)
                {
                    json_t* j_tv = json_string(optarg);
                    json_object_set_new(jcur, key, j_tv);
                    key = nullptr;
                }
                else
                {
                    jgb_warning("no key");
                }
            }
            else if(json_is_array(jcur))
            {
                json_t* j_tv = json_string(optarg);
                json_array_append_new(jcur, j_tv);
            }
            else
            {
                jgb_assert(0);
            }
            break;

        case 'U':
            if(!parent.empty())
            {
                jcur = parent.top();
                parent.pop();
            }
            else
            {
                jgb_warning("no parent");
            }
            break;

        default:
            break;
        }
    }

    jgb_info("{ interval = %f }", interval);

    char* req_str = json_dumps(jreq, JSON_COMPACT);
    jgb_debug("%s", req_str);

    std::string url = "ws://" + std::string(server) + ":" + std::string(port);

    jgb::core::get_instance()->install("wsapp", &wsapp);
    jgb::core::get_instance()->install("ws_client", &ws_client);

    int r;
    jgb::config* conf;

    r = jgb::core::get_instance()->root_conf()->get("ws_client", &conf);
    if(!r)
    {
        conf->create("protocol", protocol);
        conf->create("url", url);
        conf->create("interval", interval * 1000);
        conf->create("loop", loop);
        conf->create("req", req_str);
        conf->create("count", 0);
        conf->create("type", "ws_client_callback");
    }
    else
    {
        jgb_assert(0);
    }

    // synthesis
    client_factory::get_instance()->install("ws_client_callback", ws_client_callback_factory::get_instance());

    run();

    while(!exit_flag)
    {
        if(!loop)
        {
            if(conf->int64("count") > 0)
            {
                break;
            }
            jgb::sleep(100);
        }
        else
        {
            sleep(1);
        }
    }
    stop();

    jgb::core::get_instance()->uninstall_all();

    json_decref(jreq);
    free(req_str);

    return 0;
}
