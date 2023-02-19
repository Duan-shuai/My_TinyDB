
#ifndef CONDITION_H
#define CONDITION_H

#include <string>
#include "config.h"
#include "value.h"
/**
 * condition类 用于判断条件是否正确
 */
namespace db {
    class Condition {
    public:
        Condition() = default;

        Condition(const std::string &col_name, char op, std::string &num) {
            switch (op) {
                case '>':
                    conditionType_ = GREATER;
                    break;
                case '<':
                    conditionType_ = LESS;
                    break;
                case '=':
                    conditionType_ = EQUAL;
                    break;
                default:
                    throw error_command("NO such compared type " + std::string(op, 1));
            }
            int num_ = std::stoi(num);
            if (num_ == 0 && (num != "0" && num != "-0")) {
                throw error_command("Can't parse \"" + num + "\" to a number");
            }
            compared_num_ = num_;
            col_name_ = col_name;
        }

        bool condition_is_true(Value v) {
            switch (conditionType_) {
                case GREATER:
                    return v.GetInt() > compared_num_;
                case LESS:
                    return v.GetInt() < compared_num_;
                case EQUAL:
                    return v.GetInt() == compared_num_;
                case NO_CONDITION:
                    return true;
            }
            return true;
        }

        CONDITION_TYPE getConditionType() const {
            return conditionType_;
        }

        int getComparedNum() const {
            return compared_num_;
        }

        [[nodiscard]] const std::string &getColName() const {
            return col_name_;
        }

    private:
        CONDITION_TYPE conditionType_{NO_CONDITION};
        int compared_num_;
        std::string col_name_;
    };

}
#endif 
