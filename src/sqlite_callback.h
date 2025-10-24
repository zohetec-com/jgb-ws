#ifndef SQLITE_CALLBACK_H
#define SQLITE_CALLBACK_H

#include "object_callback.h"
#include <sqlite3.h>

namespace ws
{

class sqlite_callback: public object_callback
{
public:
    sqlite_callback(const std::string& filename)
    : filename_(filename)
    {
    }

    int exec_sql(const std::string& sql)
    {
        jgb_debug("{ sql = %s }", sql.c_str());

        sqlite3 *db;
        int r;
        r = sqlite3_open(filename_.c_str(), &db);
        if (r != SQLITE_OK)
        {
            jgb_fail("Cannot open database: %s", sqlite3_errmsg(db));
            return JGB_ERR_FAIL;
        }
        char *errMsg = 0;
        r = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
        if (r != SQLITE_OK)
        {
            jgb_fail("SQL error: %s", errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return JGB_ERR_FAIL;
        }
        sqlite3_close(db);
        return 0;\
    }

    int process(connection_context&, request& req, response& resp) override
    {
        jgb_debug("{ method = %d }", req.get_method());
        switch (req.get_method())
        {
            case request::method_e::READ:
            {
                read(req.object(), resp);
                return 0;
            }
            case request::method_e::CREATE:
            {
                create(req, resp);
                return 0;
            }
            case request::method_e::UPDATE:
            {
                update(req, resp);
                return 0;
            }
            case request::method_e::DELETE:
            {
                delete_(req, resp);
                return 0;
            }
            default:
                resp.not_implemented();
                return 0;
        }
    }

    std::string filename_;

private:

    int delete_(ws::request& req, ws::response& resp)
    {
        jgb_mark();
        int r;
        jgb::value* val;
        r = req.c->get("data", &val);
        if(!r && val->len_ > 0)
        {
            std::ostringstream os;
            for(int i=0; i<val->len_; i++)
            {
                jgb::config* obj = val->conf_[i];
                jgb::value* id_val;
                r = obj->get("id", &id_val);
                if(!r)
                {
                    os << "DELETE FROM " << req.object() << " WHERE id = ";
                    if(id_val->type_ == jgb::value::data_type::integer)
                    {
                        os << id_val->int64() << ";";
                    }
                    else if(id_val->type_ == jgb::value::data_type::string)
                    {
                        os << "'" << id_val->str() << "';";
                    }
                    else
                    {
                        jgb_assert(0);
                    }
                }
                else
                {
                    jgb_assert(0);
                }
            }
            std::string sql = os.str();
            exec_sql(sql);
        }
        resp.ok();
        jgb_mark();
        return 0;
    }

    int create(ws::request& req, ws::response& resp)
    {
        int r;
        jgb::value* val;
        r = req.c->get("data", &val);
        // todo: 使用 schema 检查 req。
        if(!r && val->len_ > 0)
        {
            std::ostringstream os;
            for(int i=0; i<val->len_; i++)
            {
                os << "INSERT INTO " << req.object() << " (";
                jgb::config* obj = val->conf_[i];
                int n = 0;
                std::ostringstream cols;
                std::ostringstream vals;
                for(auto& pr: obj->pair_)
                {
                    if(n > 0)
                    {
                        cols << ", ";
                        vals << ", ";
                    }
                    cols << pr->name_;
                    switch(pr->value_->type_)
                    {
                        case jgb::value::data_type::string:
                        case jgb::value::data_type::object:
                        vals << "'" << pr->value_ << "'";
                            break;
                        case jgb::value::data_type::integer:
                        case jgb::value::data_type::real:
                            vals << pr->value_;
                            break;
                        default:
                            //jgb_assert(0);
                            break;
                    }
                    ++ n;
                }
                os << cols.str() << ") VALUES (" << vals.str() << ");";
            }
            std::string sql = os.str();
            exec_sql(sql);
        }
        resp.ok();
        return 0;
    }

    int update(ws::request& req, ws::response& resp)
    {
        int r;
        jgb::value* val;
        r = req.c->get("data", &val);
        // todo: 使用 schema 检查 req。
        if(!r && val->len_ > 0)
        {
            std::ostringstream os;
            os << "UPDATE " << req.object() << " SET ";
            for(int i=0; i<val->len_; i++)
            {
                jgb::config* obj = val->conf_[i];
                jgb::value* id_val;
                r = obj->get("id", &id_val);
                if(!r)
                {
                    int n = 0;
                    for(auto& pr: obj->pair_)
                    {
                        if(strcmp(pr->name_, "id"))
                        {
                            if(n > 0)
                            {
                                os << ", ";
                            }
                            os << pr->name_ << " = ";
                            switch(pr->value_->type_)
                            {
                                case jgb::value::data_type::string:
                                case jgb::value::data_type::object:
                                os << "'" << pr->value_ << "'";
                                    break;
                                case jgb::value::data_type::integer:
                                case jgb::value::data_type::real:
                                    os << pr->value_;
                                    break;
                                default:
                                    //jgb_assert(0);
                                    break;
                            }
                            ++ n;
                        }
                    }
                    os << " WHERE id = ";
                    if(id_val->type_ == jgb::value::data_type::integer)
                    {
                        os << id_val->int64() << ";";
                    }
                    else if(id_val->type_ == jgb::value::data_type::string)
                    {
                        os << "'" << id_val->str() << "';";
                    }
                    else
                    {
                        jgb_assert(0);
                    }
                    std::string sql = os.str();
                    exec_sql(sql);
                }
                else
                {
                    jgb_warning("no id");
                }
            }
        }

        resp.ok();
        return 0;
    }

    int read(const std::string& table, ws::response& resp)
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

} // namespace ws

#endif // SQLITE_CALLBACK_H
