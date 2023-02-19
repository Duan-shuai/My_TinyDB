
#pragma once
#ifndef PHASER_H
#define PHASER_H

#include <string>
#include <vector>
#include <algorithm>
#include <memory>

#include "exception.h"
#include "config.h"
#include "statement.h"
#include "value.h"
namespace db {


    class Parser {
    public:
        static auto get_token(std::string &command, std::vector<std::string> &tokens) -> void;

        static auto parse_sql(std::string &sql_command, SQL_type &sqlType) -> bool;

        static auto parse_sql(Statement &statement) -> Statement *;

        static auto parse_create(Statement &statement) -> Create_Statement *;

        static auto parse_insert(Statement &statement) -> Insert_Statement *;

        static auto parse_select(Statement &statement) -> Select_Statement *;

        static auto parse_drop(Statement &statement) -> Drop_Statement *;

        static auto parse_delete(Statement &statement) -> Delete_Statement *;

        static auto parse_use(Statement &statement) -> Use_Statement *;

        static auto parse_help(Statement &statement) -> Help_Statement *;

        static auto parse_show(Statement &statement) -> Show_Statement *;

        static auto is_token(char &c) -> bool;

        static auto is_space(char &c) -> bool;

        static auto spilt(std::string command, std::vector<std::string> &values) -> void;

        static auto str_to_value(TYPE_ID typeId, std::string str) -> Value;

    private:
        /**
         * 去除最后的分号
         * @param command 一条SQL语句
         */
        static auto get_rid_last_sem(std::string &command) -> void;
    };

} 

#endif 
