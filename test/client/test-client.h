#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

#include "client_callback.h"
#include "message.h"
#include <vector>

class test_client: public client_callback
{
public:

    virtual void on_session() override
    {
        jgb::sleep(1000);
        sent_ = true;
        request_to_send();
    }

    void send(ws::request& req)
    {
        std::string str = req.to_string();
        client_callback::send(str);
    }

    virtual void on_send() override
    {
        if(sent_)
        {
            switch(step_)
            {
                case 0:
                case 4:
                case 8:
                case 12:
                {
                    ws::request req("places", "read", id_++);
                    send(req);
                    ++ step_;
                }
                break;
                case 2:
                {
                    if(place_ids_.size() > 0)
                    {
                        ws::request req("places", "delete", id_++);
                        jgb::value* val = new jgb::value(jgb::value::data_type::object, place_ids_.size(), true);
                        int idx = 0;
                        for(auto i: place_ids_)
                        {
                            val->conf_[idx]->create("id", i);
                            ++ idx;
                        }
                        req.c->create("data", val);
                        send(req);
                    }
                    else
                    {
                        jgb_debug("skip delete");
                    }
                    ++ step_;
                }
                break;
            case 6:
                {
                    ws::request req("places", "create", id_++);
                    jgb::value* val = new jgb::value(jgb::value::data_type::object, 2, true);
                    val->conf_[0]->create("id", 1);
                    val->conf_[0]->create("name", "海淀");
                    val->conf_[1]->create("id", 2);
                    val->conf_[1]->create("name", "昌平");
                    req.c->create("data", val);
                    send(req);
                    ++ step_;
                }
                break;
            case 10:
                {
                    ws::request req("places", "update", id_++);
                    jgb::value* val = new jgb::value(jgb::value::data_type::object, 1, true);
                    val->conf_[0]->create("id", 1);
                    val->conf_[0]->create("name", "朝阳");
                    req.c->create("data", val);
                    send(req);
                    ++ step_;
                    break;
                }
            }
            sent_ = false;
        }
    }

    virtual void on_recv(void *in, int len)
    {
        jgb_raw("%.*s\n", len, (char*)in);
        ws::response resp(in, len);
        jgb_assert(resp.status() == 200);
        if(step_ == 1)
        {
            place_ids_.clear();
            jgb::value* val;
            int r;
            r = resp.c->get("data", &val);
            if(!r && val->len_ > 0 && val->type_ == jgb::value::data_type::object)
            {
                for(int i = 0; i < val->len_; i++)
                {
                    jgb::config* obj = val->conf_[i];
                    int64_t id_val;
                    r = obj->get("id", id_val);
                    if(!r)
                    {
                        place_ids_.push_back(id_val);
                    }
                    else
                    {
                        jgb_assert(0);
                    }
                }
            }
        }
        ++ step_;
        if(step_ > 13)
        {
            step_ = 0;
        }
    }

    test_client(jgb::config* conf)
        : client_callback(conf),
        sent_(false),
        step_(0),
        id_(1000)
    {
    }

private:
    bool sent_;
    // 0 发查询
    // 1 等待查询
    // 2 发删除
    // 3 等待删除结果
    // 4 发查询
    // 5 等待查询
    // 6 发创建
    // 7 等待创建结果
    // 8 发查询
    // 9 等待查询
    // 10 发更新
    // 11 等待更新结果
    // 12 发查询
    // 13 等待查询
    int step_;
    int64_t id_;
    std::vector<int64_t> place_ids_;
};

#endif // TEST_CLIENT_H
