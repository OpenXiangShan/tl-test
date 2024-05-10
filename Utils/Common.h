//
// Created by Kaifan Wang on 2021/10/27.
//
// Modified by Haonan Ding on 2024/04/11
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
#include <execinfo.h>

#include "../Base/TLGlobal.hpp"

#include "gravity_utility.hpp"
#include "assert.hpp"


enum {
    DATASIZE = 64, // Cache line is 64B
    BEATSIZE = 32,
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

template<std::size_t N>
inline shared_tldata_t<N> make_shared_tldata_zero() noexcept
{
    auto data = make_shared_tldata<N>();
    std::memset(data->data, 0, DATASIZE);
    return data;
}

//

typedef uint64_t paddr_t;

#define tlc_assert(cond, ctx, info) \
    do { \
        if (!(cond)) { \
            const TLLocalContext* __tlc_assert__ctx = ctx; /* suppress the warning: -Wnonnull-compare */ \
            if (__tlc_assert__ctx) \
            { \
                std::cout << "[" << ctx->cycle() << "] [tl-test-new-ERROR] [tlc_assert failure at " << __FILE__ << ":" << __LINE__ << "] " << std::endl \
                << "[" << ctx->cycle() << "] [tl-test-new-ERROR] [At system #" << ctx->sysId() << "] "; \
            } \
            std::cout << "[tl-test-new-ERROR] " << "info: " << info << "" << std::endl; \
            std::cout << "[tl-test-new-ERROR] " << "stack backtrace: " << "" << std::endl; \
            { \
                size_t bktr_i, bktr_size; \
                void* array[1024]; \
                bktr_size = backtrace(array, 1024); \
                char** bktr_strings = backtrace_symbols(array, bktr_size); \
                for (bktr_i = 0; bktr_i < bktr_size; bktr_i++) \
                    std::cout << "#" << bktr_i << " " << bktr_strings[bktr_i] << std::endl; \
                free(bktr_strings); \
            } \
            fflush(stdout); \
            fflush(stderr); \
            TLAssertFailureEvent assert_event(__PRETTY_FUNCTION__, info); \
            assert_event.Fire(); \
            throw TLAssertFailureException(assert_event); \
            assert(false); \
        } \
    } while (0)

#define tlsys_assert(cond, info) \
    do { \
        if (!(cond)) { \
            std::cout << "[tl-test-new-ERROR] tlsys_assert() failure at " << __PRETTY_FUNCTION__ << ":" << __LINE__ << " " << std::endl; \
            std::cout << "[tl-test-new-ERROR] " << info << "" << std::endl; \
            fflush(stdout); \
            fflush(stderr); \
            TLAssertFailureEvent assert_event(__PRETTY_FUNCTION__, info); \
            assert_event.Fire(); \
            throw TLAssertFailureException(assert_event); \
            assert(false); \
        } \
    } while (0)


#define Log(ctx, str_app) \
    do { \
        if (glbl.cfg.verbose) { \
            const TLLocalContext* __tlc_assert__ctx = ctx; /* suppress the warning: -Wnonnull-compare */  \
            if (__tlc_assert__ctx) \
                std::cout << "[" << ctx->cycle() << "] [tl-test-new-INFO] #" << ctx->sysId() << " "; \
            std::cout << (Gravity::StringAppender().str_app.ToString()); \
            fflush(stdout); \
            fflush(stderr); \
        } \
    } while(0)

#define LogEx(action) \
    do { \
        if (glbl.cfg.verbose) { \
            action; \
        } \
    } while(0)


#define LogInfo(time, str_app) \
    do { \
        { \
            std::cout << "[" << time << "] [tl-test-new-INFO] "; \
            std::cout << (Gravity::StringAppender().str_app.ToString()); \
            fflush(stdout); \
            fflush(stderr); \
        } \
    } while(0)

#define LogFinal(time, str_app) \
    do { \
        { \
            std::cout << "[" << time << "] [tl-test-new-FINAL] "; \
            std::cout << (Gravity::StringAppender().str_app.ToString()); \
            fflush(stdout); \
            fflush(stderr); \
        } \
    } while(0)

#define LogFatal(time, str_app) \
    do { \
        { \
            std::cout << "[" << time << "] [tl-test-new-FATAL] "; \
            std::cout << (Gravity::StringAppender().str_app.ToString()); \
            fflush(stdout); \
            fflush(stderr); \
        } \
    } while(0)

#define LogError(time, str_app) \
    do { \
        { \
            std::cout << "[" << time << "] [tl-test-new-ERROR] "; \
            std::cout << (Gravity::StringAppender().str_app.ToString()); \
            fflush(stdout); \
            fflush(stderr); \
        } \
    } while(0)


#define Debug(ctx, str_app) \
    do { \
        if (glbl.cfg.verbose) { \
            const TLLocalContext* __tlc_assert__ctx = ctx; /* suppress the warning: -Wnonnull-compare */  \
            if (__tlc_assert__ctx) \
                std::cout << "[" << ctx->cycle() << "] [tl-test-new-DEBUG] #" << ctx->sysId() << " "; \
            std::cout << (Gravity::StringAppender().str_app.ToString()); \
            fflush(stdout); \
            fflush(stderr); \
        } \
    } while(0)

#define DebugEx(action) \
    do { \
        if (glbl.cfg.verbose) { \
            action; \
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

#define DumpEx(action) \
    do { \
        if (glbl.cfg.verbose) { \
            action; \
        } \
    } while(0)

/*
#define Log(...) \
    do {} while(0)

#define Dump(...) \
    do {} while(0)
*/

#endif //TLC_TEST_COMMON_H
