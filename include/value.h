

#ifndef VALUE_H
#define VALUE_H

#include <memory>
#include <charconv>
#include <ostream>
#include <cstring>
#include "config.h"
#include "exception.h"
namespace db {
    class Value {


    public:
        Value() = default;
        Value(TYPE_ID typeId, int value) {
            value_.int32_ = value;
            value_size_ = 4;
            typeId_ = typeId;
        }

        Value(TYPE_ID typeId, const char *str, size_t len) {
            memcpy(value_.str_, str, len);
            //TODO(ANTIO2) 检查是否需要加'\0'
            value_size_ = MAX_STRING_SIZE;
            typeId_ = typeId;
        }

        /**
         * FIXME(AntiO2) 名字为Write更合适
         * 将当前value写入指定位置
         * @param data
         */
        void GetValue(char *data) {

            switch (typeId_) {
                case INT: {
                    memcpy(data, (unsigned char *) &value_, 4);
                    break;
                }
                case STRING:

                    memcpy(data, (unsigned char *) &value_, MAX_STRING_SIZE);
                    break;
            }
        }

        [[nodiscard]] uint32_t GetSize() const {
            return value_size_;
        }

        [[nodiscard]] int GetInt() const {
            return value_.int32_;
        }

        char *GetSTRING() {
            return value_.str_;
        }

        [[nodiscard]] uint32_t getValueSize() const {
            return value_size_;
        }

        [[nodiscard]] TYPE_ID getTypeId() const {
            return typeId_;
        }

        [[nodiscard]] std::string getString() const {
            std::string s = value_.str_;
            return s;
        }

        bool operator<(const Value &v2) const {

            if (typeId_ != v2.typeId_) {
                throw error_type("Can't compare");
            }
            switch (typeId_) {
                case INT:
                    return value_.int32_ < v2.GetInt();
                case STRING: {
                    throw error_type("Can't compare string");
                }
            }
        }

        bool operator>(const Value &v2) {

            if (typeId_ != v2.typeId_) {
                throw error_type("Can't compare");
            }
            switch (typeId_) {
                case INT:
                    return value_.int32_ > v2.GetInt();
                case STRING: {
                    throw error_type("Can't compare string");
                }
            }
        }

        bool operator==(const Value &v2) {

            if (typeId_ != v2.typeId_) {
                throw error_type("Can't compare");
            }
            switch (typeId_) {
                case INT:
                    return value_.int32_ == v2.GetInt();
                case STRING: {
                    throw error_type("Can't compare string");
                }
            }
        }

        bool operator==(const Value &v2) const {

            if (typeId_ != v2.typeId_) {
                throw error_type("Can't compare");
            }
            switch (typeId_) {
                case INT:
                    return value_.int32_ == v2.GetInt();
                case STRING: {
                    throw error_type("Can't compare string");
                }
            }
        }

        explicit operator bool() const {
            return strcmp(value_.str_, "");
        }

        friend std::ostream &operator<<(std::ostream &os, const Value &value) {
            switch (value.getTypeId()) {
                case INT:
                    os << value.GetInt();
                case STRING:
                    os << value.getString();
            }
            return os;
        }

    private:
        union {
            int int32_;
            char str_[MAX_STRING_SIZE];
        } value_{};
        uint32_t value_size_;
        TYPE_ID typeId_;
    };
}


#endif //VALUE_H
