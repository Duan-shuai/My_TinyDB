
#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>
#include <fstream>
#include <sys/stat.h>
#include "config.h"

namespace db {

    class DiskManager {
    public:
        explicit DiskManager(const std::string &file_name);

        DiskManager() = default;

        virtual ~DiskManager() = default;

        virtual void ShutDown();

        virtual void WritePage(page_id_t page_id, const char *page_data);

        virtual void ReadPage(page_id_t page_id, char *page_data);

    protected:
        static auto GetFileSize(const std::string &file_name) -> int;

        std::fstream file_io_;
        std::string file_name_;
        int num_flushes_{0};
        int num_writes_{0};

    };

} 

#endif 
