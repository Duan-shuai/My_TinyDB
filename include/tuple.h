//
// Created by Anti on 2022/11/29.
//

#ifndef TUPLE_H
#define TUPLE_H

#include <vector>
#include "value.h"
#include "schema.h"
#include <cstring>
#include <ostream>

namespace db {

    class Tuple {
    public:
        Tuple(uint32_t size) {
            data_ = malloc(size);
            memset(data_, 0, size);
            size_ = size;
        }

        /**
         * 将value序列化为data
         * @param values
         * @param schema
         */
        Tuple(std::vector<Value> &values, Schema &schema) {
            size_ = schema.GetSize();
            data_ = malloc(size_);
            memset(data_, 0, size_);
            uint32_t offset = 0;
            for (auto value: values) {
                value.GetValue((char *) data_ + offset);
                offset += value.GetSize();
            }
        };

        ~Tuple() {
            free(data_);
            data_ = nullptr;
        }

        /**
         * 将当前tuple写入dst中
         * @param dst
         */
        void write(char *dst) const {
            memcpy(dst, data_, size_);
        }

        /**
         * 从src读取指定信息
         * @param src
         */
        void read(char *src) {
            memcpy(data_, src, size_);
        }

        void deserialize(std::vector<Value> &values, Schema &schema) {
            auto offset = 0;
            for (const auto &col: schema.cols_) {
                switch (col.type_) {
                    case INT: {
                        int integer;
                        memcpy((unsigned char *) &integer, (char *) data_ + offset, 4);
                        values.emplace_back(INT, integer);
                        offset += 4;
                        break;
                    }
                    case STRING: {
                        values.emplace_back(STRING, (char *) data_ + offset, MAX_STRING_SIZE);
                        offset += MAX_STRING_SIZE;
                        break;
                    }

                }
            }
        }

        friend std::ostream &operator<<(std::ostream &os, const Tuple &tuple) {
            os << "size_: " << tuple.size_ << " data_: " << tuple.data_;
            return os;
        }

    private:
        uint32_t size_;
        void *data_{nullptr};
    };
} 

#endif //TUPLE_H



