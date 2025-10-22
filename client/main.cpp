#include <csignal>
#include <jgb/config.h>
#include <jgb/log.h>
#include <jgb/core.h>
#include <jgb/helper.h>

extern jgb_api_t wsapp;
extern jgb_api_t ws_client;

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

int main()
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

    jgb::core::get_instance()->install("wsapp", &wsapp);
    jgb::core::get_instance()->install("ws_client", &ws_client);

    int r;
    jgb::config* conf;

    r = jgb::core::get_instance()->root_conf()->get("ws_client", &conf);
    if(!r)
    {
        conf->create("protocol", "ws-sync");
        conf->create("url", "ws://127.0.0.1:8000");
    }
    else
    {
        jgb_assert(0);
    }

    run();

    while(!exit_flag)
    {
        sleep(1);
    }
    stop();

    jgb::core::get_instance()->uninstall_all();

    return 0;
}
