

#include <string>
#include <cstring>
#include "disk_manager.h"
#include "exception.h"

namespace db {

    /**
     * 打开指定的文件名字
     * @param file_name
     */
    DiskManager::DiskManager(const std::string &file_name) : file_name_(file_name) {
        auto dot = file_name.rfind('.');
        if (dot == std::string::npos) {
            throw error_file("Filename Format Error");
        }
        file_io_.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
        if (!file_io_.is_open()) {
            file_io_.clear();
            file_io_.open(file_name, std::ios::binary | std::ios::trunc | std::ios::out | std::ios::in);
            if (!file_io_.is_open()) {
                throw error_file("Can't Open DataBase File");
            }
        }
    }

    /**
     * 将指定的page_id,写入磁盘中
     * @param page_id
     * @param page_data
     */
    void DiskManager::WritePage(page_id_t page_id, const char *page_data) {
        int offset = page_id * DB_PAGE_SIZE;
        file_io_.seekp(offset).write(page_data, DB_PAGE_SIZE);
        if (file_io_.bad()) {
            throw error_file("Error while writing");
        }
        file_io_.flush();
    }

    /**
     * 获得文件大小
     * @param file_name
     * @return
     */
    auto DiskManager::GetFileSize(const std::string &file_name) -> int {
        struct stat stat_buf{};
        int rc = stat(file_name.c_str(), &stat_buf);//通过文件名获取信息
        return rc == 0 ? static_cast<int>(stat_buf.st_size) : -1;
    }

    void DiskManager::ShutDown() {
        file_io_.close();
    }

    /**
     * 从文件中读入指定的页
     * @param page_id
     * @param page_data
     */
    void DiskManager::ReadPage(page_id_t page_id, char *page_data) {
        int offset = page_id * DB_PAGE_SIZE;
        if (offset > GetFileSize(file_name_)) {
            throw error_file("Error: Try to read over end of file");
        } else {
            file_io_.seekp(offset);
            file_io_.read(page_data, DB_PAGE_SIZE);
            if (file_io_.bad()) {
                throw error_file("Error: error while reading db file");
            }
            auto read_count = file_io_.gcount();
            if (read_count < DB_PAGE_SIZE) {
                file_io_.clear();
                memset(page_data + read_count, 0, DB_PAGE_SIZE - read_count);
            }
        }
    }
} 