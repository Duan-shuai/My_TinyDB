
#ifndef COLUMN_H
#define COLUMN_H
#include <string>
#include <iostream>
#include <fstream>
#include "config.h"
namespace db {

    class Column {
        friend class Table;
        friend class Schema;

    public:
        Column() = default;

        Column(TYPE_ID type, std::string &col_name, bool is_primary) {
            type_ = type;
            col_name_ = col_name;
            is_primary_=is_primary;
            if(type == INT) {
                col_size_=4;
            }
            if(type == STRING) {
                col_size_ = 256;
            }
        }

        Column(TYPE_ID type,std::string& col_name) {
            type_=type;
            col_name_=col_name;
        }

        auto GetType() -> TYPE_ID {
            return type_;
        }

        auto GetColumnSize() const -> uint32_t {
            return col_size_;
        }

        auto GetColumnName() -> std::string {
            return col_name_;
        }

        auto Is_Primary() -> bool {
            return is_primary_;
        };

        /**
         * 以下是读写column的内容
         * @param os
         * @param column
         * @return
         */
        friend std::ostream &operator<<(std::ostream &os, const Column &column) {
            os << column.type_ << " " << column.col_name_ << " "
               << column.is_primary_ << " " << column.col_size_;
            return os;
        }

        friend std::ifstream &operator>>(std::ifstream &os, Column &column) {
            unsigned int type;
            os >> type;
            column.type_ = static_cast<TYPE_ID> (type);
            os >> column.col_name_ >> column.is_primary_ >> column.col_size_;
            return os;
        }

        /**
         * TODO(ANTI) Make it private later
         */
        TYPE_ID type_;
        std::string col_name_;
        bool is_primary_{false};
        uint32_t col_size_{0};
    private:

    };
}
#endif 
