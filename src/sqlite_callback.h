#ifndef SQLITE_CALLBACK_H
#define SQLITE_CALLBACK_H

#include "object_callback.h"
#include <sqlite3.h>

namespace wsobj
{

static int sqlite_read_callback(void *data, int argc, char **argv, char **azColName)
{
    std::list<jgb::value*>* res = static_cast<std::list<jgb::value*>*>(data);
    jgb::object* obj = new jgb::object();
    for(int i = 0; i < argc; i++)
    {
        if(argv[i])
        {
            obj->set(azColName[i], jgb::value(std::string(argv[i])));
        }
        else
        {
            obj->set(azColName[i], jgb::value());
        }
    }
    res->push_back(obj);
    return 0;
}

class sqlite_callback: public object_callback
{
public:
    sqlite_callback(const std::string& db)
    : db_(db),
      db_handle_(nullptr)
    {
    }

    int process(connection_context& ctx, request& req, response& resp) override
    {
        switch (req.get_method())
        {
            case request::method_e::READ:
            {
                // Implement your SQLite read logic here
                resp.ok();
                return 0;
            }
            case request::method_e::CREATE:
            case request::method_e::UPDATE:
            case request::method_e::DELETE:
            {
                // Implement your SQLite write logic here
                resp.ok();
                return 0;
            }
            default:
                resp.not_implemented();
                return 0;
        }
    }

    std::string db_;

private:

    int open_db()
    {
        int r;
        r = sqlite3_open(db_.c_str(), &db_handle_);
        if(r)
        {
            sqlite3_close(db_handle_);
            db_handle_ = nullptr;
        }
        return r;
    }

    int read_db(const std::string& table, wsobj::response& resp)
    {
        jgb_assert(db_handle_);
        std::string sql = "SELECT * FROM " + table + ";";
        std::list<jgb::value*> res;
        char *errmsg = 0;
        int r;
        r = sqlite3_exec(db_handle_, sql.c_str(), sqlite_read_callback, &res, &errmsg);
        return r;
    }

    void close_db()
    {
        jgb_assert(db_handle_);
        sqlite3_close(db_handle_);
    }

    sqlite3* db_handle_;
};

} // namespace wsobj

#endif // SQLITE_CALLBACK_H
