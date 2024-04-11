//
// Created by Kaifan Wang on 2021/10/27.
//

#ifndef TLC_TEST_COMMON_H
#define TLC_TEST_COMMON_H

#include <memory>
#include <cmath>
#include <string>
#include <cstring>
#include <iostream>
#include <assert.h>
#include <unistd.h>

#include "../Base/TLGlobal.hpp"

#include "gravity_utility.hpp"

#ifdef ENABLE_CHISEL_DB
extern void init_db(bool en, bool select_enable, const char *select_db);
extern void save_db(const char * filename);
#endif


inline char* logdb_filename(time_t t) {
    static char buf[1024];
    char buf_time[64];
    strftime(buf_time, sizeof(buf_time), "%F@%T", localtime(&t));
    char *pwd = getcwd(NULL, 0);
    assert(pwd != NULL);
    int len = snprintf(buf, 1024, "%s/%s", pwd, buf_time);
    strcpy(buf + len, ".db");
    return buf;
}

enum {
    DATASIZE = 64, // Cache line is 64B
    BEATSIZE = 32,
    NR_ULAGENTS = 0,
    NR_CAGENTS = 1,
    NR_SOURCEID = 16,
    TIMEOUT_INTERVAL = 5000
};

//
template<std::size_t N>
using tldata_t          = uint8_t[N];

template<std::size_t N>
class wrapped_tldata_t {
    /* NOTICE: It's disappointing that GCC-11 still doen't obtain full C++20 library support.
    *          And C++20 support is still not that general on today's machine. :(
    *          So we need this wrapper to pseudoly implement C++20 shared_ptr feature.
    */
public:
    tldata_t<N> data;

public:
    operator uint8_t*() noexcept { return data; }
    operator const uint8_t*() const noexcept { return data; };
    operator tldata_t<N>&() noexcept { return data; }
    operator const tldata_t<N>&() const noexcept { return data; }
    uint8_t& operator[](size_t index) noexcept { return data[index]; };
    uint8_t  operator[](size_t index) const noexcept { return data[index]; };
};

template<std::size_t N>
using shared_tldata_t   = std::shared_ptr<wrapped_tldata_t<N>>;

template<std::size_t N>
inline shared_tldata_t<N> make_shared_tldata() noexcept
{
    return std::make_shared<wrapped_tldata_t<N>>();
}

//

typedef uint64_t paddr_t;

#ifdef ENABLE_CHISEL_DB
#define tlc_assert(cond, info) \
    do { \
        if (!(cond)) { \
            printf("\33[1;34m%s\n", info); \
            printf("Cycles: %ld\33[0m\n", Cycles); \
            if(dump_db) { \
                time_t now = time(NULL); \
                save_db(logdb_filename(now)); \
            } \
            fflush(stdout); \
            fflush(stderr); \
            assert(cond); \
        } \
    } while (0)
#else
#define tlc_assert(cond, ctx, info) \
    do { \
        if (!(cond)) { \
            TLLocalContext* __tlc_assert__ctx = ctx; /* suppress the warning: -Wnonnull-compare */ \
            if (__tlc_assert__ctx) \
            { \
                std::cout << "[tl-test-passive-ERROR] [tlc_assert failure at " << __FILE__ << ":" << __LINE__ << "] " << std::endl \
                << "[tl-test-passive-ERROR] [At cycle " << ctx->cycle() << " in system #" << ctx->sysId() << "] "; \
            } \
            std::cout << "[tl-test-passive-ERROR] " << info << "" << std::endl; \
            fflush(stdout); \
            fflush(stderr); \
            assert(cond); \
        } \
    } while (0)
#endif

#define Log(ctx, str_app) \
    do { \
        if (glbl.cfg.verbose) { \
            TLLocalContext* __tlc_assert__ctx = ctx; /* suppress the warning: -Wnonnull-compare */  \
            if (__tlc_assert__ctx) \
                std::cout << "[tl-test-passive-INFO] #" << ctx->sysId() << " "; \
            std::cout << (Gravity::StringAppender().str_app.ToString()); \
            fflush(stdout); \
            fflush(stderr); \
        } \
    } while(0)

#define Dump(str_app) \
    do { \
        if (glbl.cfg.verbose) { \
            std::cout << (Gravity::StringAppender().str_app.ToString()); \
            fflush(stdout); \
            fflush(stderr); \
        } \
    } while(0)

/*
#define Log(...) \
    do {} while(0)

#define Dump(...) \
    do {} while(0)
*/

#endif //TLC_TEST_COMMON_H
