//
// Created by Anti on 2022/12/1.
//
#include <memory>
#include <cstring>
#include <filesystem>
#include "table.h"
#include "exception.h"

namespace db {
    /**
     * 通过schema创建一个tuple
     * 这个也可以通过dat文件创建
     * @param schema
     */
    Table::Table(const Schema &schema, const std::string &dbname) {
        for (auto &page: pages) {
            page = nullptr;
        }
        schema_ = schema;
        table_name_ = schema.table_name_;
        tuple_per_page_ = DB_PAGE_SIZE / schema.GetSize();
        tuple_max_num_ = tuple_per_page_ * TABLE_MAX_PAGE;
        db_name_ = dbname;
        diskManager_ = new DiskManager(DATA_PATH + "/" + dbname + "/" + schema_.table_name_ + DATA_FORMAT);
        if (schema_.Has_Primary()) {
            /**
             * 如果有主键，则创建b+树
             */
            bpt = std::make_unique<bplus_tree>(
                    (DATA_PATH + "/" + db_name_ + "/" + schema_.table_name_ + IDX_FORMAT).c_str());
        }
    }

    Table::~Table() {
        page_id_t pageId = 0;
        for (; pageId < TABLE_MAX_PAGE; pageId++) {
            if (pages[pageId] != nullptr) {
                diskManager_->WritePage(pageId, (char *) pages[pageId]);
                free(pages[pageId]);
            }
        }
        std::ofstream ofs;
        ofs.open(DATA_PATH + "/" + db_name_ + "/" + schema_.table_name_ + INFO_FORMAT, std::ios::trunc | std::ios::out);
        ofs << schema_ << std::endl << cnt_tuple_ << " " << spare_tuple_.size() << std::endl;
        for (auto tid: spare_tuple_) {
            ofs << tid << " ";
        }
        ofs.close();
        diskManager_->ShutDown();
        delete diskManager_;

    }

    tuple_id_t Table::WriteTuple(const Tuple &tuple) {
        char *dst;
        tuple_id_t tid;
        if (spare_tuple_.empty()) {
            if (cnt_tuple_ >= tuple_max_num_) {
                throw error_table("DB:Table Overflow");
            } else {
                tid = cnt_tuple_;
                dst = LocateTuple(cnt_tuple_);
                cnt_tuple_++;
            }
        } else {
            tid = Pop_TID();
            dst = LocateTuple(tid);
        }
        tuple.write(dst);
        return tid;
    }
    char *Table::LocateTuple(const uint32_t &RID) {
        if (RID >= tuple_max_num_) {
            throw error_table("DB:Table Overflow");
        }
        auto page_id = RID / tuple_per_page_;
        if (pages[page_id] == nullptr) {
            pages[page_id] = malloc(DB_PAGE_SIZE);
            diskManager_->ReadPage(page_id, (char *) pages[page_id]);
        }
        if (pages[page_id] == nullptr) {
            throw error_table("Memory Over Flow!");
        }

        auto tuple_offset_in_page = RID % tuple_per_page_;//tuple在页内的偏移数
        auto tuple_offset_byte = tuple_offset_in_page * schema_.GetSize();
        return (char *) pages[page_id] + tuple_offset_byte;
    }

    void Table::ReadTuple(Tuple &tuple, const tuple_id_t tupleId) {
        if (tupleId >= cnt_tuple_) {
            throw error_table("DB: Try Read tuple which is Out of Range!!!plz check it ");
        }
        tuple.read((char *) LocateTuple(tupleId));
    }
    const Schema &Table::getSchema() const {
        return schema_;
    }

    std::string Table::getTableName() const {
        return table_name_;
    }

    uint32_t Table::getCntTuple() const {
        return cnt_tuple_;
    }

    uint32_t Table::getTuplePerPage() const {
        return tuple_per_page_;
    }

    uint32_t Table::getTupleMaxNum() const {
        return tuple_max_num_;
    }

    std::ostream &operator<<(std::ostream &os, const Table &table) {
        os << table.schema_;
        return os;
    }

    std::istream &operator>>(std::istream &is, Table &table) {
        return is;
    }

    void Table::addSpareTID(tuple_id_t tid) {
        spare_tuple_.insert(tid);
    }
    /**
     * 获取一个空闲的tid
     * @return
     */
    tuple_id_t Table::Pop_TID() {
        tuple_id_t tid = *spare_tuple_.begin();
        spare_tuple_.erase(spare_tuple_.begin());
        return tid;
    }

    void Table::setCntTuple(uint32_t cntTuple) {
        cnt_tuple_ = cntTuple;
    }

    bool Table::is_spare(tuple_id_t tid) {
        return spare_tuple_.find(tid) != spare_tuple_.end();
    }

    void Table::Parse_tuple(std::vector<Value> &values, Tuple &tuple) {
        tuple.deserialize(values, schema_);
    }

    void Table::Insert_Key(key_t &key, tuple_id_t &tid) const {
        bpt->insert(key, tid);
    }

    uint32_t Table::getRealTuple() const {
        return cnt_tuple_ - spare_tuple_.size();
    }

    /**
     * CHECK 这里没有删除索引中的值，注意在执行器里面remove
     * @param tid
     * @return
     */
    bool Table::delete_tuple(tuple_id_t &tid) {
        auto it = spare_tuple_.find(tid);
        if (it != spare_tuple_.end()) {
            return false;
        }
        spare_tuple_.emplace(tid);
        return true;
    }

    bool Table::exist_primary_key(key_t key) {
        value_t vt;
        if (bpt->search(key, vt) == -1)
            return false;
        return true;
    }


    std::ifstream &operator>>(std::ifstream &is, Table &table) {
        is >> table.schema_;
        return is;
    }
} 