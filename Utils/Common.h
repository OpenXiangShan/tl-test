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

//

typedef uint64_t paddr_t;

#define tlc_assert(cond, ctx, info) \
    do { \
        if (!(cond)) { \
            const TLLocalContext* __tlc_assert__ctx = ctx; /* suppress the warning: -Wnonnull-compare */ \
            if (__tlc_assert__ctx) \
            { \
                std::cout << "[" << ctx->cycle() << "] [tl-test-passive-ERROR] [tlc_assert failure at " << __FILE__ << ":" << __LINE__ << "] " << std::endl \
                << "[" << ctx->cycle() << "] [tl-test-passive-ERROR] [At system #" << ctx->sysId() << "] "; \
            } \
            std::cout << "[tl-test-passive-ERROR] " << info << "" << std::endl; \
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
                std::cout << "[" << ctx->cycle() << "] [tl-test-passive-INFO] #" << ctx->sysId() << " "; \
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
                std::cout << "[" << ctx->cycle() << "] [tl-test-passive-DEBUG] #" << ctx->sysId() << " "; \
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
