
#include <utility>

#include "database.h"

namespace db {
    Database::Database(const std::string &db_name) {
        db_name_ = db_name;
        if (std::filesystem::exists(DATA_PATH + db_name) && std::filesystem::exists(DATA_PATH + db_name))//如果已经存在文件夹
        {
            db_stream_.open(DATA_PATH + db_name + DB_INFO, std::fstream::in);
            if (!db_stream_.is_open()) {
                db_stream_.clear();
                db_stream_.open(DATA_PATH + db_name + DB_INFO, std::ios::in | std::ios::trunc | std::ios::out);
            }
            std::string table_name;
            while (!db_stream_.eof()) {
                db_stream_ >> table_name;
                if (table_name.empty()) {
                    break;
                }
                table_set_.insert(table_name);
            }
        } else {

            std::filesystem::create_directories(DATA_PATH + db_name);
            db_stream_.open(DATA_PATH + db_name + DB_INFO, std::ios::in | std::ios::trunc | std::ios::out);
        }
    }

    const std::string &Database::getDbName() const {
        return db_name_;
    }

    void Database::setDbName(const std::string &dbName) {
        db_name_ = dbName;
    }

    /**
 * 查找数据库中是否有某个表名
 * @param table
 * @return
 */
    bool Database::find(const std::string &table) noexcept {
        return !(table_set_.find(table) == table_set_.end());
    }

    Database::~Database() {
        for (auto &m: table_map_) {
            delete m.second;
            m.second = nullptr;
        }
        db_stream_.close();
        db_stream_.open(DATA_PATH + db_name_ + DB_INFO, std::ios::trunc | std::ios::out);//重新输出表名
        for (const auto &it: table_set_) {
            db_stream_ << it << std::endl;
        }
        db_stream_.close();
        table_set_.clear();
    }

    void Database::insert(const std::string &table_name) {
        if (find(table_name)) {
            throw error_table("Table " + table_name + " already exists");
        }
        table_set_.insert(table_name);
        db_stream_ << table_name << std::endl;
    }

    void Database::addTable(Table *table) {
        std::string table_name = table->getTableName();
        insert(table_name);
        table_map_.insert(std::make_pair(table_name, table));
    }

    std::ostream &operator<<(std::ostream &os, const Database &database) {
        os << "db_name_: " << database.db_name_ << " is_used_: " << database.is_used_;
        return os;
    }

    auto Database::getTable(const std::string &table) -> Table * {
        auto it = table_map_.find(table);
        if (it == table_map_.end()) {
            return nullptr;
        }
        return it->second;
    }

    void Database::recover() {
        std::string Database_path = DATA_PATH + db_name_ + "/";
        for (const auto &table_info: table_set_) {
            /**
             * 通过读取schema信息，获取table
             */
            std::ifstream ifs;
            /**
             * 哈哈，我是猪
             */
            ifs.open(Database_path + table_info + INFO_FORMAT);
            Schema schema;
            ifs >> schema;
            auto table = new Table(schema, db_name_);
            ifs >> table->cnt_tuple_;
            int spare_num = 0;
            ifs >> spare_num;
            uint32_t tid;
            while (spare_num) {
                ifs >> tid;
                table->addSpareTID(tid);
                spare_num--;
            }
            ifs.close();
            table_map_.insert(std::make_pair(table_info, table));
        }
    }

    void Database::removeTable(const std::string &table_name) {
        if (!find(table_name)) {
            throw error_table("No Table: " + table_name + " in database:" + db_name_);
        }
        table_set_.erase(table_name);
        auto it = table_map_.find(table_name);
        delete it->second;
        table_map_.erase(it);
        std::filesystem::remove(DATA_PATH + db_name_ + "/" + table_name + INFO_FORMAT);
        std::filesystem::remove(DATA_PATH + db_name_ + "/" + table_name + DATA_FORMAT);
        std::filesystem::remove(DATA_PATH + db_name_ + "/" + table_name + IDX_FORMAT);
    }

    const std::set<std::string> &Database::getTableSet() const {
        return table_set_;
    }
} // db