

#ifndef SERVER_H
#define SERVER_H

#include <string>
#include "database.h"
#include "executor.h"
namespace db {

    class Server {
    public:
        Server() = default;

        void OpenServer();

        static auto ExecuteOneSQL(std::string &&sql, std::unique_ptr<Database> *db) -> bool;
    };

} 

#endif //SERVER_H
