
#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <string>
#include <memory>
#include "config.h"
#include "table.h"
#include "statement.h"
#include "tuple.h"
#include "database.h"
#include "parser.h"
namespace db {

    class AbstractExecutor {
    };

    class CreateExecutor : AbstractExecutor {
    public:
        static auto Create(Create_Statement *createStatement, std::unique_ptr<Database> *db) -> void;

        static auto CreateDataBase(const std::string &database_name) -> bool;

        static auto CreateTable(Create_Statement *createStatement, std::unique_ptr<Database> *db) -> Table *;
    };

    class UseExecutor : AbstractExecutor {
    public:
        [[nodiscard]]static auto UseDataBase(const std::string &database_name) -> std::unique_ptr<Database>;
    };

    class InsertExecutor : AbstractExecutor {
    public:
        static auto InsertByStmt(Insert_Statement *i_stmt, std::unique_ptr<Database> *db) -> void;

        /**
         * 在指定位置写入tuple数据
         * @param tuple
         * @param
         * @return
         */
        static auto WriteTuple(Tuple &tuple, Table &table) -> void;

        static auto WriteTuple(Tuple &tuple, Table *table) -> void;
    };

    class SelectExecutor : AbstractExecutor {
    public:
        /**
         * select,并且打印出结果
         * @return
         */
        static auto Projection(std::vector<std::vector<Value>> &values, uint32_t col_id) -> void;

        //static auto OutputProjection(Select_Statement *s_stmt,std::vector<std::vector<Value>> &values)->void ;
        static auto Select(Select_Statement *s_stmt, std::unique_ptr<Database> *db) -> std::vector<std::vector<Value>>;
        /**
         * 根据tid将tuple读入，已经放弃使用
         * @param t
         * @param tid
         * @param tuple
         * @return
         */
//        static auto ReadTuple(Table *t, tuple_id_t tid,Tuple*tuple) -> bool;
    };

    class DropExecutor : AbstractExecutor {
    public:
        static auto DropTable(const std::string &table_name, std::unique_ptr<Database> *db) -> void;

        /**
         * 
         * @param db_name
         */
        static auto DropDatabase(const std::string &db_name) -> void;
    };

    class DeleteExecutor : AbstractExecutor {
    public:
        static auto DeleteByStmt(Delete_Statement *deleteStatement, std::unique_ptr<Database> *db) -> void;
    };

    class PrintExecutor : AbstractExecutor {
    public:
        static auto PrintValue(Select_Statement *s_stmt, std::vector<std::vector<Value>> &&values_s) -> void;
    };

    class HelpExecutor : AbstractExecutor {
    public:
        static auto GetHelp(Help_Statement *helpStatement) -> void;

        static auto SimpleHelp() -> void;

        static auto InsertHelp() -> void;

        static auto CreateHelp() -> void;

        static auto SelectHelp() -> void;

        static auto DeleteHelp() -> void;

        static auto DropHelp() -> void;

        static auto UseHelp() -> void;

        static auto ExitHelp() -> void;

        static auto ShowHelp() -> void;
    };

    class ShowExecutor : AbstractExecutor {
    public:
        static auto ShowTable(std::unique_ptr<Database> *db) -> void;

        static auto ShowDatabase() -> void;
    };
} 

#endif 
