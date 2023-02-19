
#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <ostream>
#include "trie.h"
#include "config.h"
#include "exception.h"
#include "table.h"

namespace db{

    class Database {
    public:
        Database() : is_used_(false) {};

        explicit Database(const std::string &db_name);

        ~Database();

        [[nodiscard]] const std::string &getDbName() const;

        void setDbName(const std::string &dbName);

        /**
         * 查找数据库中是否有某个表名
         * @param table
         * @return
         */
        bool find(const std::string &table) noexcept;

        void insert(const std::string &table_name);

        void addTable(Table *table);

        void removeTable(const std::string &table_name);

        void recover();

        friend std::ostream &operator<<(std::ostream &os, const Database &database);

        auto getTable(const std::string &table) -> Table *;

        const std::set<std::string> &getTableSet() const;

    private:
        std::string db_name_;
        std::set<std::string> table_set_;
        std::fstream db_stream_;
        bool is_used_{true};
        std::map<std::string, Table *> table_map_;


    };

} 

#endif 