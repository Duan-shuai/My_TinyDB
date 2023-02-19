

#include <filesystem>
#include <memory>
#include "executor.h"
#include "exception.h"
#include "table.h"
#include <climits>
namespace db {
    /**
     * @param createStatement
     */
    auto CreateExecutor::Create(Create_Statement *createStatement, std::unique_ptr<Database> *db) -> void {
        if (createStatement->createType_ == CREATE_DATABASE) {
            CreateDataBase(createStatement->name_);
        }
        CreateTable(createStatement, db);
    }

    auto CreateExecutor::CreateDataBase(const std::string &database_name) -> bool {
        if (database_name.empty()) {
            return false;
        }
        if (std::filesystem::exists(DATA_PATH + database_name) && std::filesystem::exists(DATA_PATH + database_name)) {
            throw error_command("DataBase " + database_name + " exists");
        }
        auto db = new Database(database_name);
        delete db;
        return true;
    }

    auto
    CreateExecutor::CreateTable(Create_Statement *createStatement, std::unique_ptr<Database> *db) -> Table * {
        auto table = new Table(createStatement->schema_, db->get()->getDbName());
        /**
         * 如果重名，则抛出
        */
        try {
            db->get()->addTable(table);
        }
            /**
             * CHECK 检查这里重名文件是否会受到影响
             *
             */
        catch (error_table &e) {
            delete table;
            throw e;
        }
        /**
         * create table 还需要记录元信息
         */
        std::fstream fsm;
        fsm.open(DATA_PATH + db->get()->getDbName() + "/" + createStatement->schema_.table_name_ + ".info",
                 std::ios::trunc | std::ios::out);
        if (!fsm.is_open()) {
            throw error_file("Cant open information file : " + createStatement->schema_.table_name_ + ".info");
        }
        fsm << createStatement->schema_;
        fsm.flush();
        fsm.close();
        return table;
    }

    /**
     * 判断数据库是否存在
     * @param database_name
     * @return
     */
    auto UseExecutor::UseDataBase(const std::string &database_name) -> std::unique_ptr<Database> {
        if (!std::filesystem::exists(DATA_PATH + database_name)) {
            throw error_command("DataBase \"" + database_name + "\" doesn't exist!!!");
        }
        auto db = std::make_unique<Database>(database_name);
        db->recover();
        return db;
    }

    /**
     * 将tuple写入指定位置
     * @param tuple
     * @param dst
     * @return
     */
    auto InsertExecutor::WriteTuple(Tuple &tuple, Table &table) -> void {
        table.WriteTuple(tuple);
    }

    auto InsertExecutor::WriteTuple(Tuple &tuple, Table *table) -> void {
        table->WriteTuple(tuple);
    }

    auto InsertExecutor::InsertByStmt(Insert_Statement *i_stmt, std::unique_ptr<Database> *db) -> void {
        auto table_name = i_stmt->table_name_;
        auto table = db->get()->getTable(table_name);
        if (table == nullptr) {
            throw error_table("No table named " + table_name);
        }
        /**
         * 将value转化为tuple
         */
        auto schema = table->getSchema();
        std::vector<Value> values;
        for (auto i = 0; i < schema.cols_.size(); i++) {
            switch (schema.cols_[i].type_) {
                case INT:
                    values.emplace_back(Parser::str_to_value(INT, i_stmt->value_str[i]));
                    break;
                case STRING:
                    values.emplace_back(Parser::str_to_value(STRING, i_stmt->value_str[i]));
                    break;
            }
        }
        if (schema.Has_Primary()) {
            /**
             * FIXME() 这里可能出现主键重复的情况
             *
             */
            int key = values.at(schema.getKeyId()).GetInt();

            if (table->exist_primary_key(key)) {
                throw error_table("Primary key " + std::to_string(key) + " already exists in table " + table_name);
            }
            auto tid = table->WriteTuple(Tuple(values, schema));
            table->Insert_Key(key, tid);
        } else {
            table->WriteTuple(Tuple(values, schema));
        }

    }


//    auto SelectExecutor::ReadTuple(Table *t, const tuple_id_t tid) -> Tuple {
//        std::vector<Value> values;
//        Tuple tuple(t->getSchema().GetSize());
//        if (t->is_spare(tid)) {
//            throw error_table("Inner error:Try read deleted tuple.Please Connect antio2@qq.com");
//        }
//        t->ReadTuple(tuple, tid);
//        return tuple;
//    }

    [[nodiscard]]auto
    SelectExecutor::Select(Select_Statement *s_stmt, std::unique_ptr<Database> *db) -> std::vector<std::vector<Value>> {
        std::vector<std::vector<Value>> results;
        if (db == nullptr) {
            throw error_database("No database using");
        }
        if (*db == nullptr) {
            throw error_database("No database using");
        }
        auto table = db->get()->getTable(s_stmt->table_name_);
        if (table == nullptr) {
            throw error_table("No table named " + s_stmt->table_name_);
        }
        /**
         * 如果没有主键,遍历所有tuple 或者有主键但是没有条件
         * 或者是有主键，有条件，但是需要比较的列和主键不是同一列
         */
        auto tuple_size = table->getSchema().GetSize();
        if (!table->getSchema().Has_Primary() || (table->getSchema().Has_Primary() && (!s_stmt->has_condition ||
                                                                                       (s_stmt->has_condition &&
                                                                                        (s_stmt->condition.getColName() !=
                                                                                         table->getSchema().cols_.at(
                                                                                                 table->schema_.getKeyId()).col_name_))))) {

            for (auto i = 0; i < table->getCntTuple(); i++) {
                if (table->is_spare(i)) {
                    continue;
                }
                Tuple tuple(tuple_size);
                table->ReadTuple(tuple, i);
                std::vector<Value> vs;
                table->Parse_tuple(vs, tuple);
                if (s_stmt->has_condition) {
                    /**
                     * FIXME(AntiO2) 想到了一个问题，如果列的名字相同呢？parser是没有处理这种情况的
                     *
                     * 解决方案：在table中创建一个parser的map，这样由解决了寻找col_name的方式，又能找出col_name相同的情况
                     */
                    auto col_condition = table->schema_.GetColId(s_stmt->condition.getColName());
                    if (col_condition == -1) {
                        throw error_command("No column " + s_stmt->condition.getColName());
                    }
                    /**
                     * 如果不满足条件，删除
                     */
                    if (!s_stmt->condition.condition_is_true(vs[col_condition])) {
                        continue;
                    }
                }
                results.emplace_back(vs);
            }
        } else {
            key_t left = INT_MIN;
            key_t right = INT_MAX;

            tuple_id_t tid{0};
            std::vector<tuple_id_t> tids_;
            switch (s_stmt->condition.getConditionType()) {

                case NO_CONDITION:
                    break;
                case GREATER: {
                    auto left_ = s_stmt->condition.getComparedNum();
                    table->bpt->search_range(&left_, right, tids_, (size_t) right - (size_t) left_ + 1);
                    break;
                }
                case LESS: {
                    auto right_ = s_stmt->condition.getComparedNum() - 1;
                    table->bpt->search_range(&left, right_, tids_, (size_t) right_ - (size_t) left + 1);
                    break;
                }

                case EQUAL: {
                    int flag = table->bpt->search(s_stmt->condition.getComparedNum(), tid);
                    if (flag != -1) {
                        tids_.push_back(tid);
                    }
                }


                    break;
            }
            /**
             * 根据tuple id,寻找对应的tuple
             * FIXME 这里和上面的代码有重复，考虑优化为函数
             */
            for (auto a_tid: tids_) {
                if (table->is_spare(a_tid)) {
                    continue;
                }
                Tuple tuple(tuple_size);
                table->ReadTuple(tuple, a_tid);
                std::vector<Value> vs;
                table->Parse_tuple(vs, tuple);
                results.emplace_back(vs);
            }
        }
        /**
         * 这里对结果做投影操作，并提供选取列的信息
         */
        if (!s_stmt->select_all_) {
            auto col_selected = table->schema_.GetColId(s_stmt->col_name_);
            if (col_selected == -1) {
                throw error_command("No column " + s_stmt->col_name_);
            }
            s_stmt->selected_cols.emplace_back(s_stmt->col_name_, table->schema_.cols_[col_selected].type_);
            Projection(results, col_selected);
        } else {
            for (const auto &col: table->schema_.cols_) {
                s_stmt->selected_cols.emplace_back(col.col_name_, col.type_);
            }
        }

        return results;
    }

    auto SelectExecutor::Projection(std::vector<std::vector<Value>> &values, uint32_t col_id) -> void {
        for (auto &tuple_value: values) {
            auto selected_value = tuple_value[col_id];
            tuple_value.clear();
            tuple_value.emplace_back(selected_value);
        }
    }



    auto DropExecutor::DropTable(const std::string &table_name, std::unique_ptr<Database> *db) -> void {
        if (db == nullptr) {
            throw error_database("No database selected");
        }
        if (*db == nullptr) {
            throw error_database("No database using");
        }
        db->get()->removeTable(table_name);
    }

    auto DropExecutor::DropDatabase(const std::string &db_name) -> void {
        if (std::filesystem::remove_all(DATA_PATH + db_name)) {
            return;
        }
        throw error_database("Can't find database " + db_name);
    }

    auto DeleteExecutor::DeleteByStmt(Delete_Statement *deleteStatement, std::unique_ptr<Database> *db) -> void {
        if (db == nullptr) {
            throw error_database("No database using");
        }
        if (*db == nullptr) {
            throw error_database("No database using");
        }
        auto table = db->get()->getTable(deleteStatement->table_name_);
        if (table == nullptr) {
            throw error_table("No table named " + deleteStatement->table_name_);
        }
        /**
         * 如果没有主键,遍历所有tuple 或者有主键但是没有条件
         * 或者是有主键，有条件，但是需要比较的列和主键不是同一列
         *
         * 简而言之就是需要遍历tid
         */
        auto tuple_size = table->getSchema().GetSize();
        auto has_primary = table->schema_.Has_Primary();
        auto primary_col_id = table->schema_.getKeyId();
        /**
         *
         * 如果有判断条件，获得条件列
         */
        int col_condition = -1;

        if (deleteStatement->has_condition) {
            col_condition = table->schema_.GetColId(deleteStatement->condition.getColName());
            if (col_condition == -1) {
                throw error_command("No column " + deleteStatement->condition.getColName());
            }
        }
        if (!has_primary || ((!deleteStatement->has_condition || (deleteStatement->has_condition &&
                                                                  (deleteStatement->condition.getColName() !=
                                                                   table->getSchema().cols_.at(
                                                                           table->schema_.getKeyId()).col_name_))))) {

            for (tuple_id_t i = 0; i < table->getCntTuple(); i++) {
                if (table->is_spare(i)) {
                    continue;
                }
                Tuple tuple(tuple_size);
                table->ReadTuple(tuple, i);
                std::vector<Value> vs;
                table->Parse_tuple(vs, tuple);
                /**
                 *
                 */
                if (deleteStatement->has_condition) {
                    if (!deleteStatement->condition.condition_is_true(vs[col_condition])) {
                        continue;
                    }
                }
                /**
                 * 如果有主键，删去对应索引中的键值对
                 */
                if (has_primary) {
                    table->bpt->remove(vs.at(primary_col_id).GetInt());
                }
                table->delete_tuple(i);
            }
        } else {
            key_t left = INT_MIN;
            key_t right = INT_MAX;

            tuple_id_t tid{0};
            std::vector<tuple_id_t> tids_;
            switch (deleteStatement->condition.getConditionType()) {

                case NO_CONDITION:
                    break;
                case GREATER: {
                    auto left_ = deleteStatement->condition.getComparedNum();
                    table->bpt->search_range(&left_, right, tids_, (size_t) right - (size_t) left_ + 1);
                    break;
                }
                case LESS: {
                    auto right_ = deleteStatement->condition.getComparedNum() - 1;
                    table->bpt->search_range(&left, right_, tids_, (size_t) right_ - (size_t) left + 1);
                    break;
                }

                case EQUAL: {
                    int flag = table->bpt->search(deleteStatement->condition.getComparedNum(), tid);
                    if (flag != -1) {
                        tids_.push_back(tid);
                    }
                }
            }
            /**
             * 根据tuple id,寻找对应的tuple
             * FIXME 这里和上面的代码有重复，考虑优化为函数
             */
            for (auto a_tid: tids_) {
                /**
                 * CHECK(这里已经删除的索引不会出现tids_中)
                 * 调试时打断点优化
                 */
                if (table->is_spare(a_tid)) {
                    continue;//xxx
                }
                /**
                 * FIXME 感觉这里有点蠢，根据主键找到了范围内的tid，再根据tid解析出满足条件主键，传入remove删除
                 * 原因是bpt中没有实现按照范围删除
                 * 后期优化加上
                 */
                Tuple tuple(tuple_size);
                table->ReadTuple(tuple, a_tid);
                std::vector<Value> vs;
                table->Parse_tuple(vs, tuple);
                table->bpt->remove(vs.at(primary_col_id).GetInt());
                table->delete_tuple(a_tid);
            }
        }
    }


    auto PrintExecutor::PrintValue(Select_Statement *s_stmt, std::vector<std::vector<Value>> &&values_s) -> void {
        auto col_num = s_stmt->selected_cols.size();
        for (auto &col_name: s_stmt->selected_cols) {
            std::cout << std::setw(10) << std::left << col_name.first.c_str();
        }
        printf("\n------------------------------------\n");
        for (auto &values: values_s) {
            for (auto i = 0; i < col_num; i++) {
                switch (s_stmt->selected_cols.at(i).second) {
                    case INT: {
                        auto integer = values.at(i).GetInt();
                        std::cout << std::setw(std::max(10, (int) std::to_string(integer).length())) << std::left
                                  << integer;
                        break;
                    }


                    case STRING:
                        auto str = values.at(i).getString();
                        std::cout << std::setw((int) str.length() + 1) << std::left << str;
                        break;
                }
            }
            printf("\n");
        }
    }

    auto HelpExecutor::GetHelp(Help_Statement *helpStatement) -> void {
        switch (helpStatement->helpType) {
            case NONE:
                SimpleHelp();
                break;
            case INSERT:
                InsertHelp();
                break;
            case CREATE:
                CreateHelp();
                break;
            case SELECT:
                SelectHelp();
                break;
            case DELETE:
                DeleteHelp();
                break;
            case DROP:
                DropHelp();
                break;
            case USE:
                UseHelp();
                break;
            case EXIT:
                ExitHelp();
                break;
            case HELP:
                SimpleHelp();
                break;
            case SHOW:
                ShowHelp();
                break;
        }
        std::cout << std::endl;
    }

    auto HelpExecutor::SimpleHelp() -> void {
        std::cout << "This is a simple guide for you to use SimpleDB\n"
                     "SQL sentence is required in typical format"
                     "\n------------------------------\n"
                     "And it is very important to exit SimpleDB by command \"exit\"!!!"
                     "\n------------------------------\n"
                     "Type in \"help <option>\" to learn more\n"
                     "Here are options you can use\n"
                     "insert\tcreate\tselect\tdelete\tdrop\tuse\texit\tshow\t";
    }

    auto HelpExecutor::InsertHelp() -> void {
        std::cout << "<usage>insert <table> values (<const-value>[, <const-value>…])\n"
                     "You can't use default value\n"
                     "Before INSERT, you should use a database";
    }

    auto HelpExecutor::CreateHelp() -> void {
        std::cout << "Create SQL can create new database or new table\n"
                     "<usage>create database <database_name>\n"
                     "<usage>create table <table-name> (\n"
                     "<column> <type> [primary], \n"
                     "…)\n"
                     "Before create a table, you should use a database\n";
    }

    auto HelpExecutor::SelectHelp() -> void {
        std::cout << "select <column> from <table> [ where <cond> ]\n"
                     "<column>： <column-name> | *\n"
                     "* means select all columns\n"
                     "where is an optional sentence"
                     "\"where\"'s format is [col_name op const_value\n"
                     "SimpleDB only support compare integer\n"
                     "[op]:< or = or >";
    }

    auto HelpExecutor::DeleteHelp() -> void {
        std::cout << "delete <table> [ where <cond> ]\n"
                     "delete <table> without condition means delete all tuples\n"
                     "where is an optional sentence"
                     "\"where\"'s format is [col_name op const_value\n"
                     "SimpleDB only support compare integer\n"
                     "[op]:< or = or >";
    }

    auto HelpExecutor::DropHelp() -> void {
        std::cout << "<usage>drop database <database name>\n"
                     "<usage>drop table <table-name>\n"
                     "You can't delete the database which you are using now";
    }

    auto HelpExecutor::UseHelp() -> void {
        std::cout << "<usage>use <dbname>\n";
    }

    auto HelpExecutor::ExitHelp() -> void {
        std::cout << "<usage>exit\n"
                     "It's necessary that quit SimpleDB by this command";
    }

    auto HelpExecutor::ShowHelp() -> void {
        std::cout << "<usage>show database\n"
                     "Display database's names thar are on your disk\n"
                     "<usage>show table\n"
                     "show table's names in the database which you are using";
    }

    auto ShowExecutor::ShowDatabase() -> void {
        if (!(std::filesystem::exists(DATA_PATH) && std::filesystem::is_directory(DATA_PATH))) {
            return;
        }
        auto list = std::filesystem::directory_iterator(DATA_PATH);
        for (auto &file: list) {
            if (std::filesystem::is_directory(file))
                std::cout << file.path().filename() << std::endl;
        }
    }

    auto ShowExecutor::ShowTable(std::unique_ptr<Database> *db) -> void {
        if (*db == nullptr) {
            throw error_database("No selected database");
        }
        for (auto table: db->get()->getTableSet()) {
            std::cout << table << std::endl;
        }
    }
} 