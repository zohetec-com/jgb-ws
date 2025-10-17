#ifndef SYNC_SERVICE_CLIENT_H
#define SYNC_SERVICE_CLIENT_H

#include "connection_callback.h"
#include <jgb/log.h>
#include <jgb/config_factory.h>
#include <jgb/config.h>
#include "helper.h"

static const char* stage_names [] = {
    "idle",
    "httping",
    "wsing"
};

static const char* ws_stage_names [] = {
    "idle",
    "connecting",
    "connected",
    "aborting",
    "closed"
};

class sync_service_client: public connection_callback
{
public:
    static sync_service_client* get_instance()
    {
        static sync_service_client instance;
        return &instance;
    }

    static void print_global(jgb::config* c)
    {
        int r;
        int iv;
        r = c->get("loaded", iv);
        if(!r)
        {
            jgb_raw("2. 加载矫正数据：%s\n", iv ? "成功" : "失败");
        }
        else
        {
            jgb_error("get loaded. { r = %d }", r);
        }
    }

    static void print_response(jgb::config* c)
    {
        int r;
        jgb::value* val;

        r =  c->get("instances", &val);
        if(!r)
        {
            jgb_raw("4. 应答：\n");
            jgb_raw("Board               IP |     command code           mts | command code           mts\n");
            jgb_assert(val->type_ == jgb::value::data_type::object);
            for(int i=0; i<val->len_; i++)
            {
                jgb::config* info = val->conf_[i];
                int board_id = info->int64("board");

                jgb_raw("%5d", board_id);
                jgb_raw(" %16s", info->str("ip").c_str());
                std::string cmd;
                r = info->get("sync_img_cfg/response/command", cmd);
                if(!r)
                {
                    jgb_raw(" %13s", get_basename(cmd).c_str());
                    jgb_raw(" %4s", info->str("sync_img_cfg/response/code").c_str());
                    jgb_raw(" %13d", info->int64("sync_img_cfg/response/mts"));
                    r = info->get("isp_bin_cfg/response/command", cmd);
                    if(!r)
                    {
                        jgb_raw(" %9s", get_basename(cmd).c_str());
                        jgb_raw(" %4s", info->str("isp_bin_cfg/response/code").c_str());
                        jgb_raw(" %13d", info->int64("isp_bin_cfg/response/mts"));
                    }
                    else
                    {
                        jgb_raw(" %9s", "-");
                        jgb_raw(" %4s", "-");
                        jgb_raw(" %13s", "-");
                    }
                }
                jgb_raw("\n");
            }
        }
    }

    static void print_connect(jgb::config* c)
    {
        int r;
        jgb::value* val;

        r =  c->get("instances", &val);
        if(!r)
        {
            jgb_raw("3. 连接：\n");
            jgb_raw("Board               IP   Stage         WS |     NTP.type NTP.enable NTP.status            Datetime\n");
            jgb_assert(val->type_ == jgb::value::data_type::object);
            for(int i=0; i<val->len_; i++)
            {
                jgb::config* info = val->conf_[i];
                int board_id = info->int64("board");

                jgb_raw("%5d", board_id);
                jgb_raw(" %16s", info->str("ip").c_str());
                jgb_raw(" %7s", stage_names[info->int64("stage")]);
                jgb_raw(" %10s", ws_stage_names[info->int64("ws_stage")]);
                jgb_raw(" %1s", "");
                jgb_raw(" %12s", info->str("get_ntp_cfg/result/ntp_cfg/type").c_str());
                jgb_raw(" %10d", info->int64("get_ntp_cfg/result/ntp_cfg/enable"));
                int status = -1;
                r = info->get("get_dev_info/result/image_info/status", status);
                jgb_raw(" %10d", status);
                jgb_raw(" %19s", info->str("time").c_str());
                jgb_raw("\n");
            }
        }
    }

    static void print_isp_status_sd3403(jgb::config* c)
    {
        int r;
        jgb::value* val;

        r =  c->get("instances", &val);
        if(!r)
        {
            jgb_raw("1. ISP 状态：\n");
            jgb_raw("Board Camra Ae Awb Rgain Ggain Bgain Expo. Again Dgain ISPgain  Luma   ISO Temp ISPbin\n");
            jgb_assert(val->type_ == jgb::value::data_type::object);
            for(int i=0; i<val->len_; i++)
            {
                int board_id = val->conf_[i]->int64("board");
                jgb::value* info;
                r = val->conf_[i]->get("get_dev_info/result/image_info", &info);
                if(!r)
                {
                    jgb_assert(info->type_ == jgb::value::data_type::object);
                    for(int j=0; j<info->len_; j++)
                    {
                        jgb::config* c = info->conf_[j];
                        jgb_raw("%5d", board_id);
                        jgb_raw(" %5d", c->int64("id"));
                        jgb_raw("   ");
                        jgb_raw("    ");
                        jgb_raw(" %5d", c->int64("awb_r_gain"));
                        jgb_raw(" %5d", c->int64("awb_g_gain"));
                        jgb_raw(" %5d", c->int64("awb_b_gain"));
                        jgb_raw(" %5d", c->int64("ae_exp_time"));
                        jgb_raw(" %5d", c->int64("ae_a_gain"));
                        jgb_raw(" %5d", c->int64("ae_d_gain"));
                        jgb_raw(" %7d", c->int64("ae_isp_gain"));
                        jgb_raw(" %5d", c->int64("AvgLuma"));
                        jgb_raw(" %5d", c->int64("iso"));
                        jgb_raw(" %4d", c->int64("temperature"));
                        jgb_raw(" %6s", c->str("isp_bin_mode").c_str());
                        jgb_raw("\n");
                    }
                }
            }
        }
    }

    static void print_isp_status_cvcam(jgb::config* c)
    {
        int r;
        jgb::value* val;

        r =  c->get("instances", &val);
        if(!r)
        {
            jgb_raw("1. ISP 状态：\n");
            jgb_raw("Board Camra Ae Awb Rgain Ggain Bgain Expo. Again Dgain ISPgain  Light HistError Temp ISPbin\n");
            jgb_assert(val->type_ == jgb::value::data_type::object);
            for(int i=0; i<val->len_; i++)
            {
                int board_id = val->conf_[i]->int64("board");
                jgb::value* info;
                r = val->conf_[i]->get("get_dev_info/result/image_info", &info);
                if(!r)
                {
                    jgb_assert(info->type_ == jgb::value::data_type::object);
                    for(int j=0; j<info->len_; j++)
                    {
                        jgb::config* c = info->conf_[j];
                        jgb_raw("%5d", board_id);
                        jgb_raw(" %5d", c->int64("id"));
                        jgb_raw("   ");
                        jgb_raw("    ");
                        jgb_raw(" %5d", c->int64("awb_r_gain"));
                        jgb_raw(" %5d", c->int64("awb_g_gain"));
                        jgb_raw(" %5d", c->int64("awb_b_gain"));
                        jgb_raw(" %5d", c->int64("ae_exp_time"));
                        jgb_raw(" %5d", c->int64("ae_a_gain"));
                        jgb_raw(" %5d", c->int64("ae_d_gain"));
                        jgb_raw(" %7d", c->int64("ae_isp_gain"));
                        jgb_raw(" %6.2f", c->real("light_value"));
                        jgb_raw(" %9d", c->int64("hist_error"));
                        jgb_raw(" %4d", c->int64("temperature"));
                        jgb_raw(" %6s", c->str("isp_bin_mode").c_str());
                        jgb_raw("\n");
                    }
                }
            }
        }
    }

    static void print_get_dev_info(jgb::config* c)
    {
        std::string cameraType = c->str("cameraType");
        if(cameraType == "sd3403_mode1")
        {
            print_isp_status_sd3403(c);
        }
        else
        {
            print_isp_status_cvcam(c);
        }
        print_global(c);
        print_connect(c);
        print_response(c);
    }

    static void print_sync_img_cfg(jgb::config* c)
    {
        int r;
        jgb::value* val;

        r =  c->get("instances", &val);
        if(!r)
        {
            jgb_raw("5. ISP 设置：\n");
            jgb_raw("Board Camra Ae Awb Rgain Ggain Bgain Expo. Again Dgain ISPgain ISPbin SyncMode\n");
            jgb_assert(val->type_ == jgb::value::data_type::object);
            for(int i=0; i<val->len_; i++)
            {
                int board_id = val->conf_[i]->int64("board");
                jgb::value* cfg;
                r = val->conf_[i]->get("sync_img_cfg/request", &cfg);
                if(!r)
                {
                    jgb::value* cfg2;
                    r = val->conf_[i]->get("isp_bin_cfg/request", &cfg2);
                    jgb_assert(!r);
                    jgb_assert(cfg->type_ == jgb::value::data_type::object);
                    jgb_assert(cfg2->type_ == jgb::value::data_type::object);
                    for(int j=0; j<cfg->len_; j++)
                    {
                        jgb::config* c = cfg->conf_[j];
                        jgb_raw("%5d", board_id);
                        jgb_raw(" %5d", c->int64("id"));
                        jgb_raw(" %2d", c->int64("auto_ae_enable"));
                        jgb_raw(" %3d", c->int64("auto_awb_enable"));
                        jgb_raw(" %5d", c->int64("Rgain"));
                        jgb_raw(" %5d", c->int64("Ggain"));
                        jgb_raw(" %5d", c->int64("Bgain"));
                        jgb_raw(" %5d", c->int64("EXP_time"));
                        jgb_raw(" %5d", c->int64("Again"));
                        jgb_raw(" %5d", c->int64("Dgain"));
                        jgb_raw(" %7d", c->int64("ISPgain"));
                        std::string isb_bin = cfg2->conf_[j]->str("isp_bin_mode");
                        if(!isb_bin.empty())
                        {
                            jgb_raw(" %6s",  cfg2->conf_[j]->str("isp_bin_mode").c_str());
                        }
                        else
                        {
                            jgb_raw(" %6s", "-");
                        }
                        jgb_raw(" %8d ", c->int64("sync_mode"));
                        jgb_raw("\n");
                    }
                }
            }
        }
    }

    virtual void on_recv(void *in, int len)
    {
        if(dump_recv_)
        {
            jgb_raw("%.*s\n", len, (char*)in);
        }
        jgb::config* conf = jgb::config_factory::create((char*) in, len);
        print_get_dev_info(conf);
        jgb_raw("\n");
        print_sync_img_cfg(conf);
        delete conf;
    }

    bool dump_recv_;

private:
    sync_service_client()
        : dump_recv_(false)
    {}
};

#endif // SYNC_SERVICE_CLIENT_H
