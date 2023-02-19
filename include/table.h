
#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include "schema.h"
#include "config.h"
#include "tuple.h"
#include "disk_manager.h"
#include "bpt.h"

namespace db {
//    class Key_TID{
//    public:
//        Key_TID(Value key,tuple_id_t tid):key_tid(key,tid)
//        {
//        }
//        std::pair <Value ,tuple_id_t > key_tid;
//
//        bool operator==(Key_TID&keyTid2)
//        {
//            return key_tid.first==keyTid2.key_tid.first;
//        }
//        bool operator<(const Key_TID&keyTid2)const
//        {
//            return key_tid.first<keyTid2.key_tid.first;
//        }
//
//        bool operator>( Key_TID&keyTid2)
//        {
//            return key_tid.first>keyTid2.key_tid.first;
//        }
//    };
    /**
     * 表类
     */
    class Table {
    public:
        Table() = default;

        explicit Table(const Schema &schema, const std::string &dbname);

        ~Table();

        /**
         * 传入tuple，写入表中
         * @param tuple
         */
        tuple_id_t WriteTuple(const Tuple &tuple);

        /**
         * 读出指定rid
         * @param tuple
         * @param tupleId
         */
        void ReadTuple(Tuple &tuple, tuple_id_t tupleId);

        char *LocateTuple(const uint32_t &RID);

        [[nodiscard]] const Schema &getSchema() const;

        [[nodiscard]] std::string getTableName() const;

        [[nodiscard]] uint32_t getCntTuple() const;

        uint32_t getRealTuple() const;

        [[nodiscard]] uint32_t getTuplePerPage() const;

        [[nodiscard]] uint32_t getTupleMaxNum() const;

        void addSpareTID(tuple_id_t tid);

        tuple_id_t Pop_TID();

        friend std::ostream &operator<<(std::ostream &os, const Table &table);

        friend std::istream &operator>>(std::istream &is, Table &table);

        bool is_spare(tuple_id_t tid);

        /**
         * 是否存在主键
         * @param key
         * @return
         */
        bool exist_primary_key(key_t key);

        /**
         * 通过tuple生成Value向量
         * @return
         */
        void Parse_tuple(std::vector<Value> &values, Tuple &tuple);

        Schema schema_;
        uint32_t cnt_tuple_{0};

        bool delete_tuple(tuple_id_t &tid);

        void Insert_Key(key_t &key, tuple_id_t &tid) const;

        void setCntTuple(uint32_t cntTuple);

        std::unique_ptr<bplus_tree> bpt;
    private:
        std::string table_name_;
        void *pages[TABLE_MAX_PAGE]{};
        DiskManager *diskManager_;
        uint32_t tuple_per_page_;//每页存放tuple数目
        uint32_t tuple_max_num_;//最多tuple数目
        std::set<tuple_id_t> spare_tuple_;
        std::string db_name_;
    };

} 

#endif //TABLE_H
