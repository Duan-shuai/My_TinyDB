
#pragma once
#ifndef SCHEMA_H
#define SCHEMA_H

#include <utility>
#include <vector>
#include <iostream>
#include <map>
#include "column.h"
#include "config.h"

#include "exception.h"
namespace db {
    class Schema {

    public:
        Schema() = default;

        void AddCols(Column &&col) {
            auto it = col_id_.find(col.col_name_);
            if (it != col_id_.end()) {
                /**
                 *
                 * 这里说明已经加入过这个列名了！！！
                 *
                 */
                throw error_table("Column " + col.col_name_ + " already in schema");
            }
            cols_.push_back(col);
            offset_ += col.GetColumnSize();
            col_id_.emplace(col.col_name_, col_id_.size());
            if (col.Is_Primary()) {
                if (has_primary) {
                    throw error_table("Already have a primary key");
                }
                has_primary = true;
                key_id_ = col_id_.size() - 1;
            }
        }

        [[nodiscard]] int GetColId(const std::string &col_name) {
            auto it = col_id_.find(col_name);
            /**
             * CHECK 这里抛出异常会不会更好？
             * 如果没有该列，返回-1
             */
            if (it == col_id_.end()) {
                return -1;
            }
            return it->second;
        }

        [[nodiscard]] uint32_t GetSize() const {
            return offset_;
        }

        [[nodiscard]] bool Has_Primary() const {
            return has_primary;
        };

        auto Set_Primary(bool is_primary) -> void {
            has_primary = is_primary;
        }

        friend std::ostream &operator<<(std::ostream &os, const Schema &schema) {
            os << schema.table_name_ << " " << schema.cols_.size() << std::endl;
            for (const auto &col: schema.cols_) {
                os << col << std::endl;
            }
            return os;
        }

        friend std::ifstream &operator>>(std::ifstream &ifs, Schema &schema) {
            auto col_num = 0;
            ifs >> schema.table_name_ >> col_num;
            for (auto i = 0; i < col_num; i++) {
                Column col;
                ifs >> col;
                schema.AddCols(std::move(col));
            }
            return ifs;
        }

        [[nodiscard]] uint32_t getKeyId() const {
            return key_id_;
        }

        void setKeyId(uint32_t keyId) {
            key_id_ = keyId;
        }

        std::string table_name_;
        std::vector<Column> cols_;
    private:
        /**
         * key_id_就是主键所在的列号
         */
        uint32_t key_id_{0};
        uint32_t offset_{0};
        bool has_primary{false};
        std::map<std::string, uint32_t> col_id_;
    };
}

#endif 
