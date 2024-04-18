//
// Created by Kaifan Wang on 2021/10/25.
//

#include <cstddef>
#include <iterator>
#include <map>
#include <array>
#include <memory>
#include <type_traits>

#include "../Base/TLLocal.hpp"

#include "Common.h"
#include "gravity_utility.hpp"

#ifndef TLC_TEST_SCOREBOARD_H
#define TLC_TEST_SCOREBOARD_H

const int ERR_NOTFOUND = 1;
const int ERR_MISMATCH = 2;

#define SB_DEBUG        1


#ifndef SB_DEBUG
#   define SB_DEBUG        0
#endif


template<std::size_t N>
inline void data_dump(const uint8_t* data)
{
    Gravity::StringAppender sa;

    sa.Hex().Fill('0');

    for (std::size_t j = 0; j < N; j++)
        sa.NextWidth(2).Append(uint64_t(data[j]), " ");

    std::cout << sa.EndLine().ToString();
}

template<std::size_t N>
inline void data_dump_on_verify(const uint8_t *dut, const uint8_t *ref)
{
    std::cout << std::endl;

    std::cout << "dut: ";
    data_dump<N>(dut);

    std::cout << "ref: ";
    data_dump<N>(ref);
}


template<typename Tk, typename Tv>
class ScoreBoard {
protected:
    std::map<Tk, std::shared_ptr<Tv>> mapping;
public:
    ScoreBoard();
    ~ScoreBoard();
    std::map<Tk, std::shared_ptr<Tv>>& get();
    void update(const Tk& key, std::shared_ptr<Tv>& data);
    std::shared_ptr<Tv> query(TLLocalContext* ctx, const Tk& key);
    void erase(TLLocalContext* ctx, const Tk& key);
    int verify(const Tk& key, const Tv& data) const;
    bool haskey(const Tk& key);
};

class Global_SBEntry {
public:
    enum {
        SB_INVALID = 0,
        SB_VALID,
        SB_PENDING
    };
    int status;
    shared_tldata_t<DATASIZE>   data;
    shared_tldata_t<DATASIZE>   pending_data; // used for put&release
};

template<typename T>
class GlobalBoard : public ScoreBoard<T, Global_SBEntry> {
private:
    int data_check(TLLocalContext* ctx, const uint8_t* dut, const uint8_t* ref, std::string assert_info);
    uint8_t init_zeros[DATASIZE];
public:
    GlobalBoard() noexcept;
    int verify(TLLocalContext* ctx, const T& key, shared_tldata_t<DATASIZE> data);
    void unpending(TLLocalContext* ctx, const T& key);
};

/************************** Implementation **************************/

template<typename Tk, typename Tv>
ScoreBoard<Tk, Tv>::ScoreBoard() {
    mapping.clear();
}

template<typename Tk, typename Tv>
ScoreBoard<Tk, Tv>::~ScoreBoard() {
}

template<typename Tk, typename Tv>
std::map<Tk, std::shared_ptr<Tv>>& ScoreBoard<Tk, Tv>::get() {
    return this->mapping;
}

template<typename Tk, typename Tv>
void ScoreBoard<Tk, Tv>::update(const Tk& key, std::shared_ptr<Tv>& data) {

#   if SB_DEBUG == 1
        if constexpr (std::is_base_of_v<Global_SBEntry, Tv>)
        {
            std::cout << Gravity::StringAppender("[tl-test-passive-DEBUG] global scoreboard update: ")
                .ShowBase()
                .Hex().Append("key = ", uint64_t(key))
                .Dec().Append(", present = ", mapping.count(key))
                .ToString();

            std::cout << ", type = Global_SBEntry";

            std::cout << ", status = ";
            switch (data->status)
            {
                case Global_SBEntry::SB_INVALID:    std::cout << "SB_INVALID";  break;
                case Global_SBEntry::SB_VALID:      std::cout << "SB_VALID";    break;
                case Global_SBEntry::SB_PENDING:    std::cout << "SB_PENDING";  break;
                default:
                    std::cout << "<unknown:" << uint64_t(data->status) << ">";   
                    break;
            }

            std::cout << std::endl;

            std::cout << "data - data : ";
            if (data->data != nullptr)
                data_dump<DATASIZE>(data->data->data);
            else
                std::cout << "<non-initialized>" << std::endl;

            std::cout << "data - pend : ";
            if (data->data != nullptr)
                data_dump<DATASIZE>(data->data->data);
            else
                std::cout << "<non-initialized>" << std::endl;
        }
#   endif

    if (mapping.count(key) != 0) {
        mapping[key] = data;
    } else {
        mapping.insert(std::make_pair(key, data));
    }
}

template<typename Tk, typename Tv>
std::shared_ptr<Tv> ScoreBoard<Tk, Tv>::query(TLLocalContext* ctx, const Tk& key) {
    if (mapping.count(key) > 0) {
        return mapping[key];
    } else {
        tlc_assert(false, ctx, 
            Gravity::StringAppender().Hex().ShowBase()
                .Append("Key no found: ", uint64_t(key))
            .ToString());
    }
}

template<typename Tk, typename Tv>
void ScoreBoard<Tk, Tv>::erase(TLLocalContext* ctx, const Tk& key) {
    int num = mapping.erase(key);
    tlc_assert(num == 1, ctx, "Multiple value mapped to one key!");
}

template<typename Tk, typename Tv>
int ScoreBoard<Tk, Tv>::verify(const Tk& key, const Tv& data) const {
    if (mapping.count(key) > 0) {
        if (*mapping[key] != data) {
            return ERR_MISMATCH;
        }
        return 0;
    }
    return ERR_NOTFOUND;
}

template<typename Tk, typename Tv>
bool ScoreBoard<Tk, Tv>::haskey(const Tk &key) {
    return mapping.count(key) > 0;
}

template<typename T>
GlobalBoard<T>::GlobalBoard() noexcept
{
    std::memset(init_zeros, 0, DATASIZE);
}


template<typename T>
int GlobalBoard<T>::data_check(TLLocalContext* ctx, const uint8_t *dut, const uint8_t *ref, std::string assert_info) {
    for (int i = 0; i < DATASIZE; i++) {
        if (dut[i] != ref[i]) {
            data_dump_on_verify<DATASIZE>(dut, ref);
            tlc_assert(false, ctx, assert_info.data());
            return -1;
        }
    }
    return 0;
}

template<typename T>
int GlobalBoard<T>::verify(TLLocalContext* ctx, const T& key, shared_tldata_t<DATASIZE> data) {
    if (this->mapping.count(key) == 0) { // we assume data is all zero initially
        return this->data_check(ctx, data->data, init_zeros, "Init data is non-zero!");
    }
    tlc_assert(this->mapping.count(key) == 1, ctx, "Duplicate records found in GlobalBoard!");

    Global_SBEntry value = *this->mapping.at(key).get();
    if (value.status == Global_SBEntry::SB_VALID) {
        tlc_assert(value.data != nullptr, ctx, "NULL occured in valid entry of GlobalBoard!");
        return this->data_check(ctx, data->data, value.data->data, "Data mismatch from status SB_VALID!");
    } else if (value.status == Global_SBEntry::SB_PENDING) {
        bool flag = true;
        if (value.data != nullptr) {
            for (int i = 0; i < DATASIZE; i++) {
                if (data->data[i] != value.data->data[i]) {
                    flag = false;
                    break;
                }
            }
            if (flag) return 0;
        } else {
            for (int i = 0; i < DATASIZE; i++) {
                if (data->data[i] != init_zeros[i]) {
                    flag = false;
                    break;
                }
            }
            if (flag) return 0;
        }
        tlc_assert(value.pending_data != nullptr, ctx, "NULL occured in pending entry of GlobalBoard!");
        this->data_check(ctx, data->data, value.pending_data->data, "Data mismatch from status SB_PENDING!");
        return 0;
    } else {
        // TODO: handle other status
        tlc_assert(false, ctx, "Unknown GlobalBoard entry status!");
        return -1;
    }
}

template<typename T>
void GlobalBoard<T>::unpending(TLLocalContext* ctx, const T& key) {
    tlc_assert(this->mapping.count(key) == 1, ctx, "Un-pending non-exist entry in GlobalBoard!");
    Global_SBEntry* value = this->mapping.at(key).get();
    // tlc_assert(value->pending_data != nullptr, "Un-pending entry with NULL ptr in GlobalBoard!");
    if (value->pending_data == nullptr) {
        return;
    }
    value->data = value->pending_data;
    value->pending_data = nullptr;
    value->status = Global_SBEntry::SB_VALID;
}

#endif // TLC_TEST_SCOREBOARD_H
