#ifndef SQLITE_CALLBACK_H
#define SQLITE_CALLBACK_H

#include "object_callback.h"
#include <sqlite3.h>

namespace wsobj
{

class sqlite_callback: public object_callback
{
public:
    sqlite_callback(const std::string& filename)
    : filename_(filename)
    {
    }

    int process(connection_context&, request& req, response& resp) override
    {
        switch (req.get_method())
        {
            case request::method_e::READ:
            {
                read_db(req.object(), resp);
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

    std::string filename_;

private:

    int read_db(const std::string& table, wsobj::response& resp)
    {
        std::string sql = "SELECT * FROM " + table + ";";
        std::list<jgb::config*> res;
        int r;
        sqlite3 *db;

        jgb_debug("{ sql = %s }", sql.c_str());

        r = sqlite3_open(filename_.c_str(), &db);
        if (r != SQLITE_OK)
        {
            jgb_fail("Cannot open database: %s", sqlite3_errmsg(db));
            return JGB_ERR_FAIL;
        }

        sqlite3_stmt *stmt;
        r = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        if (r != SQLITE_OK) {
            jgb_fail("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return JGB_ERR_FAIL;
        }

        while ((r = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            // Iterate through rows
            jgb::config* obj = new jgb::config();
            for(int i=0; i<sqlite3_column_count(stmt); i++)
            {
                // Iterate through columns
                const char *colName = sqlite3_column_name(stmt, i);
                int dataType = sqlite3_column_type(stmt, i);
                jgb_debug("{ i = %d, colName = %s, dataType = %d }", i, colName, dataType);
                switch(dataType) {
                    case SQLITE_INTEGER:
                    obj->create(colName, (int64_t) sqlite3_column_int64(stmt, i));
                        break;
                    case SQLITE_FLOAT:
                        obj->create(colName, sqlite3_column_double(stmt, i));
                        break;
                    case SQLITE_TEXT:
                        obj->create(colName, (const char*) sqlite3_column_text(stmt, i));
                        jgb_debug("%s: %s", colName, sqlite3_column_text(stmt, i));
                        break;
                    default:
                        jgb_assert(0);
                        break;
                }
            }
            jgb_debug("%s", obj->to_string().c_str());
            res.push_back(obj);
        }

        if(r == SQLITE_DONE)
        {
            jgb_debug("%u", res.size());
            jgb::value* val = new jgb::value(jgb::value::data_type::object, res.size(), true);
            int idx = 0;
            for(auto i: res)
            {
                delete val->conf_[idx];
                val->conf_[idx] = i;
                val->conf_[idx]->id_ = idx;
                val->conf_[idx]->uplink_ = val;
                jgb_debug("%d: %s", idx, val->conf_[idx]->to_string().c_str());
                ++ idx;
            }
            resp.c->create("data", val);
            resp.ok();
        }
        else
        {
            resp.status(500);
            jgb_fail("Error during SELECT: %s\n", sqlite3_errmsg(db));
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        return r;
    }
};

} // namespace wsobj

#endif // SQLITE_CALLBACK_H
