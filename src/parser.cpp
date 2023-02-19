
#define DEBUG 0
#if DEBUG
#include "iostream"
using std::cout;
using std::endl;
#endif

#include "parser.h"


namespace db {
    char TOKEN[] = {
            ',',
            '(',
            ')',
            ';',
            '>',
            '<',
            '='
    };
    char SPACE[] = {
            '\r',
            '\n',
            ' '
    };

    /**
     * 通过string解析sql，已废弃
     * @param sql_command
     * @param sqlType
     * @return
     */
    auto Parser::parse_sql(std::string &sql_command, SQL_type &sqlType) -> bool {
        std::vector<std::string> tokens;
        get_rid_last_sem(sql_command);
        get_token(sql_command, tokens);
        std::string first_token = tokens[0];
        if (first_token == "insert") {
            sqlType = INSERT;
            return true;
        }
        if (first_token == "select") {
            sqlType = SELECT;
            return true;
        }
        if (first_token == "create") {
            sqlType = CREATE;
            return true;
        }
        if (first_token == "delete") {
            sqlType = DELETE;
            return true;
        }
        if (first_token == "exit") {
            sqlType = EXIT;
            return true;
        }
        return false;
    }

    /**
     * TODO(Anti) 用的时候记得捕获异常
     * 通过Statement解析sql语句
     * @param statement
     * @return
     */
    auto Parser::parse_sql(Statement &statement) -> Statement * {
        if (statement.commandline_.empty()) {
            statement.sqlType_ = NONE;
            return &statement;
        }
        get_rid_last_sem(statement.commandline_);
        get_token(statement.commandline_, statement.tokens);

        std::transform(statement.tokens[0].begin(),
                       statement.tokens[0].end(),
                       statement.tokens[0].begin(),
                       tolower);
        auto first_token = statement.tokens[0];
        if (first_token == "insert") {
            statement.sqlType_ = INSERT;
            return parse_insert(statement);
        }
        if (first_token == "select") {
            statement.sqlType_ = SELECT;
            return parse_select(statement);
        }
        if (first_token == "create") {
            statement.sqlType_ = CREATE;
            auto res = parse_create(statement);
            return res;
        }
        if (first_token == "delete") {
            statement.sqlType_ = DELETE;
            return parse_delete(statement);
        }
        if (first_token == "drop") {
            statement.sqlType_ = DROP;
            return parse_drop(statement);
        }
        if (first_token == "use") {
            statement.sqlType_ = USE;
            return parse_use(statement);
        }
        if (first_token == "exit") {
            statement.sqlType_ = EXIT;
            return &statement;
        }
        if (first_token == "help") {
            statement.sqlType_ = HELP;
            return parse_help(statement);
        }
        if (first_token == "show") {
            statement.sqlType_ = SHOW;
            return parse_show(statement);
        }
        throw error_command("SQL isn't correct no such type as: " + first_token);
    }

    /**
     * NOT 分割空格
     * DONE(Anti) 分割出括号，逗号
     * @param command
     * @param tokens
     */
    auto Parser::get_token(std::string &command, std::vector<std::string> &tokens) -> void {
        if (command.empty()) {
            tokens.clear();
            return;
        }
        command += " ";
        /**
         * 扫描一遍，获取token，如果是空格 跳过，单词或者指定标志符则加入tokens
         *
         *
         */
        auto point = -1;
        for (auto i = 0; i < command.length(); i++) {
            char c = command[i];
            /**
             * 如果是空格，检查前面是否有单词
             */
            if (is_space(c)) {
                if (point != -1) {
                    tokens.push_back(command.substr(point, i - point));
                    point = -1;
                }
            } else {
                /**
                 * 如果是token
                 */
                if (is_token(c)) {
                    if (point != -1) {
                        tokens.push_back(command.substr(point, i - point));
                        point = -1;
                    }
                    tokens.emplace_back(1, c);
                } else {
                    /**
                     * 如果之前没有指向单词首字母，将point放在当前位置
                     */
                    if (point == -1) {
                        point = i;
                    }
                }
            }
        }
    }

    /**
     * FIXME 可能有错
     * 去掉语句最后的分号
     * @param command
     */
    auto Parser::get_rid_last_sem(std::string &command) -> void {
        if (command.empty()) {
            return;
        }
        for (auto c = command.begin(); c != command.end(); c++) {
            if (*c == ';') {
                command = std::string(command.begin(), c);
                return;
            }
        }
    }

    auto Parser::parse_create(Statement &statement) -> Create_Statement * {
        if (statement.tokens.size() < 3) {
            throw error_command("Argument Number isn't CORRECT");
        }
        std::transform(statement.tokens[1].begin(), statement.tokens[1].end(), statement.tokens[1].begin(), toupper);
        auto createStatement = new Create_Statement(std::move(statement));
        if (createStatement->tokens[1] == "TABLE") {
            /**
             * create table table_name (  ); 至少有五个单词
             */
            if (createStatement->tokens.size() < 5) {
                throw error_command("Invalid Create Statement");
            }
            createStatement->name_ = createStatement->tokens[2];// table name
            createStatement->createType_ = CREATE_TABLE;
            createStatement->schema_.table_name_ = createStatement->name_;
            /**
             * TODO(AntiO2) 分词器 加入括号解析
             * 解析列信息
             * 创建对应schema
             *
             */
            if (createStatement->tokens[3] != "(") {
                throw error_command("No '(' Found");
            }
            if (*createStatement->tokens.rbegin() != ")") {
                throw error_command("Create Table statement should end with ')'");
            }
            Column new_col = *new Column();
            int cnt = 0;
            /**
             * CHECK(AntiO2) not right
             */
            for (int i = 4; i < createStatement->tokens.size(); i++) {

                auto token = createStatement->tokens[i];
                if (token == ")") {
                    if (cnt != 0) {
                        createStatement->schema_.AddCols(std::move(new_col));
                    }
                    break;
                }
                switch (cnt) {
                    case 0://处理列名阶段
                        new_col.col_name_ = token;
                        cnt++;

                        break;
                    case 1://处理类型阶段
                        if (token == "int") {
                            new_col.type_ = INT;
                            new_col.col_size_ = 4;

                        } else if (token == "string") {
                            new_col.type_ = STRING;
                            new_col.col_size_ = MAX_STRING_SIZE;
                        } else {
                            throw error_command("Invalid type named " + token);
                        }
                        cnt++;
                        break;
                    case 2://处理主键，或者结束一个列的定义
                        if (token == ",") {
                            createStatement->schema_.AddCols(std::move(new_col));
                            new_col = *new Column();
                            cnt = 0;
                            break;
                        }
                        if (token == "primary") {
                            if (createStatement->schema_.Has_Primary()) {
                                throw error_command("Two Primary Keys? It is not allowed by SimpleDB");
                            }
                            new_col.is_primary_ = true;
                            break;
                        }
                    default:
                        throw error_command("Invalid create table,please check it:error occurred near :" + token);
                }
            }
            return createStatement;
        }
        if (createStatement->tokens[1] == "DATABASE") {
            if (createStatement->tokens.size() != 3) {
                throw error_command("Argument Number isn't CORRECT");
            }
            createStatement->name_ = createStatement->tokens[2];
            createStatement->createType_ = CREATE_DATABASE;
            return createStatement;
        }
        throw error_command("You can't create " + createStatement->tokens[1]);
    }

    /**
     * FIXME(AntiO2)这里会出现一个问题，就是因为分词去掉了空格，string会丢失空格
     * 解决：直接解析字符串
     * @param statement
     * @return
     *
     */
    auto Parser::parse_insert(Statement &statement) -> Insert_Statement * {
        auto i_stmt = new Insert_Statement(std::move(statement));
        auto token = i_stmt->tokens;
        /**
         * insert antio2 values( )
         */
        if (i_stmt->tokens.size() < 5) {
            throw error_command("Check the command: " + i_stmt->commandline_);
        }
        i_stmt->table_name_ = i_stmt->tokens[1];
        if (i_stmt->tokens[2] != "values") {
            throw error_command("Check the command: " + i_stmt->commandline_);
        }
        if (token[3] != "(") {
            throw error_command("Check the command: " + i_stmt->commandline_);
        }
        if (token[token.size() - 1] != ")") {
            throw error_command("Check the command: " + i_stmt->commandline_);
        }
        auto l_pos = i_stmt->commandline_.find('(');
        auto r_pos = i_stmt->commandline_.rfind(')');
        auto value_str = i_stmt->commandline_.substr(l_pos + 1, r_pos - l_pos - 1);
        spilt(value_str, i_stmt->value_str);
        return i_stmt;
    }

    auto Parser::parse_select(Statement &statement) -> Select_Statement * {
        auto s_stmt = new Select_Statement(std::move(statement));
        auto tokens = s_stmt->tokens;
        if (tokens.size() < 4) {
            throw error_command("Command isn't correct: " + s_stmt->commandline_);
        }
        if (tokens[2] != "from") {
            throw error_command("Command isn't correct,require from: " + s_stmt->commandline_);
        }
        auto table_name = tokens[3];
        s_stmt->table_name_ = table_name;
        if (tokens[1] == "*") {
            s_stmt->select_all_ = true;
        } else {
            s_stmt->col_name_ = tokens[1];
        }
        /**
         * 判断条件
         */
        if (tokens.size() > 4 && tokens[4] == "where") {
            if (tokens.size() != 8) {
                throw error_command("Error when parse condition: " + s_stmt->commandline_);
            }
            s_stmt->has_condition = true;
            /**
             * FIXME 会有bug
             * 比如token[6]为 >anti
             * 解决：判断符号加入空格
             */
            s_stmt->condition = Condition(tokens[5], tokens[6][0], tokens[7]);
        }
        return s_stmt;
    }

    auto Parser::parse_use(Statement &statement) -> Use_Statement * {
        auto u_stmt = new Use_Statement(std::move(statement));
        if (u_stmt->tokens.size() != 2) {
            throw error_command("Argument isn't correct! e.g. Use <dbname>");
        }
        if (u_stmt->tokens[1].empty()) {
            throw error_command("No DB name ! e.g. Use <dbname>");
        }
        u_stmt->db_name_ = u_stmt->tokens[1];
        return u_stmt;
    }

    /**
     * delete table [where col op num]
     * 有2个token 或者 6个token
     * @param statement
     * @return
     */
    auto Parser::parse_delete(Statement &statement) -> Delete_Statement * {
        auto d_stmt = new Delete_Statement(std::move(statement));
        auto tokens = d_stmt->tokens;
        if (tokens.size() == 2) {
            d_stmt->table_name_ = tokens[1];
            return d_stmt;
        }
        if (tokens.size() != 6) {
            throw error_command("Delete command isn't correct\n<usage>delete tablename [where col_name op num]");
        }
        if (tokens[2] != "where") {
            throw error_command("Delete command isn't correct\n<usage>delete tablename [where col_name op num]");
        }
        auto table_name = tokens[1];
        d_stmt->table_name_ = table_name;
        d_stmt->has_condition = true;
        d_stmt->condition = Condition(tokens[3], tokens[4][0], tokens[5]);
        return d_stmt;
    }

    auto Parser::parse_drop(Statement &statement) -> Drop_Statement * {
        if (statement.tokens.size() != 3) {
            throw error_command("Arguments number isn't correct: " + statement.commandline_);
        }
        auto d_stmt = new Drop_Statement(std::move(statement));
        if (d_stmt->tokens[1] == "database") {
            d_stmt->dropType_ = DROP_DATABASE;
        } else if (d_stmt->tokens[1] == "table") {
            d_stmt->dropType_ = DROP_TABLE;
        } else {
            throw error_command("Can't find drop type: " + d_stmt->commandline_);
        }
        d_stmt->name_ = d_stmt->tokens[2];
        return d_stmt;
    }

    auto Parser::is_token(char &c) -> bool {
        for (const auto &token: TOKEN) {
            if (c == token) {
                return true;
            }
        }
        return false;
    }

    auto Parser::is_space(char &c) -> bool {
        for (auto space: SPACE) {
            if (c == space) {
                return true;
            }
        }
        return false;
    }

    auto Parser::spilt(std::string command, std::vector<std::string> &values) -> void {
        if (command.empty()) {
            values.clear();
            return;
        }
        auto point = 0;
        command += ",";
        bool pre_str = false;//之前的值是一个字符串
        for (auto i = 0; i < command.length(); i++) {
            auto char_ = command[i];
            if (char_ == ',' && !pre_str) {
                auto new_value = command.substr(point, i - point);
                values.emplace_back(new_value);
                point = i + 1;
                continue;
            }
            if (char_ == ',') {
                pre_str = false;
                point = i + 1;
                continue;
            }
            if (char_ == '\"') {
                if (pre_str) {
                    auto new_value = command.substr(point, i - point);
                    values.emplace_back(new_value);
                } else {
                    pre_str = true;
                    point = i + 1;
                }
            }
        }
    }

    auto Parser::str_to_value(TYPE_ID typeId, const std::string str) -> Value {
        switch (typeId) {
            case INT: {
                int num = std::stoi(str);
                if (num == 0 && (str != "0" && str != "-0")) {
                    throw error_command("Can't parse \"" + str + "\" to a number");
                }
                return {INT, num};
            }
            case STRING: {
                return {STRING, str.c_str(), str.length()};
            }
        }
        throw error_type("No such type");
    }

    auto Parser::parse_help(Statement &statement) -> Help_Statement * {
        auto h_stmt = new Help_Statement(std::move(statement));
        h_stmt->helpType = NONE;
        if (h_stmt->tokens.size() == 1 || h_stmt->tokens.size() != 2) {
            return h_stmt;
        }
        auto help = h_stmt->tokens.at(1);
        if (help == "insert") {
            h_stmt->helpType = INSERT;
        }
        if (help == "create") {
            h_stmt->helpType = CREATE;
        }
        if (help == "select") {
            h_stmt->helpType = SELECT;
        }
        if (help == "delete") {
            h_stmt->helpType = DELETE;
        }
        if (help == "drop") {
            h_stmt->helpType = DROP;
        }
        if (help == "use") {
            h_stmt->helpType = USE;
        }
        if (help == "exit") {
            h_stmt->helpType = EXIT;
        }
        if (help == "show") {
            h_stmt->helpType = SHOW;
        }
        return h_stmt;
    }

    auto Parser::parse_show(Statement &statement) -> Show_Statement * {
        auto show_stmt = new Show_Statement(std::move(statement));
        if (show_stmt->tokens.size() != 2) {
            throw error_command("Can't parse show command type: " + show_stmt->commandline_);
        }
        auto show_ = show_stmt->tokens.at(1);
        if (show_ == "table") {
            show_stmt->showType = SHOW_TABLE;
            return show_stmt;
        }
        if (show_ == "database") {
            show_stmt->showType = SHOW_DATABASE;
            return show_stmt;
        }
        throw error_command("Can't parse near: " + show_);
    }

} 