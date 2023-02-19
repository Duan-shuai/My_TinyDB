//
// Created by Anti on 2022/12/2.
//

#include <iostream>
#include <string>
#include "server.h"
#include "parser.h"
#include "statement.h"
#include "executor.h"
#include "exception.h"
namespace db {
    void Server::OpenServer() {
        std::unique_ptr<Database> db_used_ = nullptr;//现在使用的db
        while (true) {
            std::cout << "(SimpleDB)>>>";
            std::string sql;
            std::getline(std::cin, sql);
            if (!ExecuteOneSQL(std::move(sql), &db_used_)) {
                delete db_used_.get();
                db_used_ = nullptr;
                break;
            }
        }
    }

    auto Server::ExecuteOneSQL(std::string &&sql, std::unique_ptr<Database> *db) -> bool {
        Statement stmt(sql);
        try {
            auto stmt_p = Parser::parse_sql(stmt);

            switch (stmt_p->sqlType_) {
                case INSERT:
                    if (*db == nullptr) {
                        throw error_database("No database using");
                    }
                    InsertExecutor::InsertByStmt((Insert_Statement *) stmt_p, db);
                    break;
                case CREATE:
                    switch (((Create_Statement *) stmt_p)->createType_) {
                        case CREATE_DATABASE:
                            CreateExecutor::CreateDataBase(((Create_Statement *) stmt_p)->name_);
                            break;
                        case CREATE_TABLE:
                            if (db == nullptr || *db == nullptr) {
                                throw error_database("No database using");
                            }
                            CreateExecutor::CreateTable((Create_Statement *) stmt_p, db);
                    }
                    break;
                case SELECT: {
                    if (*db == nullptr) {
                        throw error_database("No database using");
                    }

                    auto results = SelectExecutor::Select((Select_Statement *) stmt_p, db);
                    PrintExecutor::PrintValue((Select_Statement *) stmt_p, std::move(results));
                    break;
                }


                case DELETE:
                    if (*db == nullptr) {
                        throw error_database("No database using");
                    }
                    DeleteExecutor::DeleteByStmt((Delete_Statement *) stmt_p, db);
                    break;
                case DROP: {
                    auto name = ((Drop_Statement *) stmt_p)->name_;
                    switch (((Drop_Statement *) stmt_p)->dropType_) {
                        case DROP_TABLE:
                            DropExecutor::DropTable(name, db);
                            break;
                        case DROP_DATABASE:
                            /**
                             * CHECK
                             * 这里不知道为什么不能使用&&来短路
                             */
                            if (db != nullptr && (*db != nullptr) && db->get()->getDbName() == name) {
                                throw error_database("Database " + name + " is now used,can't drop it!");
                            }

                            DropExecutor::DropDatabase(name);
                            break;
                    }
                    break;
                }

                case USE: {
                    std::string db_name = ((Use_Statement *) stmt_p)->db_name_;
                    *db = UseExecutor::UseDataBase(db_name);
                    break;
                }
                case HELP: {
                    HelpExecutor::GetHelp((Help_Statement *) stmt_p);
                    break;
                }
                case EXIT:
                    *db = nullptr;
                    std::cout << "Bye bye" << std::endl;
                    return false;
                case NONE: {
                    return true;
                }


                case SHOW:
                    switch (((Show_Statement *) stmt_p)->showType) {

                        case SHOW_TABLE:
                            if (db == nullptr || *db == nullptr) {
                                throw error_database("No database selected");
                            }
                            ShowExecutor::ShowTable(db);
                            break;
                        case SHOW_DATABASE:
                            ShowExecutor::ShowDatabase();
                            break;
                    }

                    break;
            }
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
            std::cout << R"(Please type in "help" or "help <command name>" for help)" << std::endl;
        }
        return true;
    }
} 