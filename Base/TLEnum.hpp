#pragma once
//
// Created by Kumonda221 (Ding Haonan) on 2024/03/20
//

#ifndef TLC_TEST_TLENUM_H
#define TLC_TEST_TLENUM_H

#include <string>

#include "../Utils/gravity_utility.hpp"


enum {
    PutFullData     = 0,
    PutPartialData  = 1,
    ArithmeticData  = 2,
    LogicalData     = 3,
    Get             = 4,
    Intent          = 5,
    AcquireBlock    = 6,
    AcquirePerm     = 7
};

enum {
    /* NOTICE: ProbePerm not implemented in L1 & L2, and never issued.
    */
    ProbeBlock      = 6,
//  ProbePerm       = 7
};

enum {
    AccessAck       = 0,
    AccessAckData   = 1,
    HintAck         = 2,
    ProbeAck        = 4,
    ProbeAckData    = 5,
    Release         = 6,
    ReleaseData     = 7
};

enum {
    Grant           = 4,
    GrantData       = 5,
    ReleaseAck      = 6
};

enum {
    GrantAck        = 0
};

enum {
    toT = 0,
    toB,
    toN
};

enum {
    NtoB = 0,
    NtoT,
    BtoT
};

enum {
    TtoB = 0,
    TtoN,
    BtoN,
    TtoT,
    BtoB,
    NtoN
};

enum {
    INVALID = 0,
    BRANCH,
    TRUNK,
    TIP
};

//
inline std::string ProbeParamToString(int param) noexcept
{
    switch (param)
    {
        case toT:       return "toT";
        case toB:       return "toB";
        case toN:       return "toN";
        default:        return Gravity::StringAppender("<unknown_probe_param:", param, ">").ToString();
    }
}

inline std::string GrantParamToString(int param) noexcept
{
    switch (param)
    {
        case toT:       return "toT";
        case toB:       return "toB";
        case toN:       return "toN";
        default:        return Gravity::StringAppender("<unknown_grant_param:", param, ">").ToString();
    }
}

inline std::string GrantDataParamToString(int param) noexcept
{
    switch (param)
    {
        case toT:       return "toT";
        case toB:       return "toB";
        case toN:       return "toN";
        default:        return Gravity::StringAppender("<unknown_grantdata_param:", param, ">").ToString();
    }
}

inline std::string AcquireParamToString(int param) noexcept
{
    switch (param)
    {
        case NtoB:      return "NtoB";
        case NtoT:      return "NtoT";
        case BtoT:      return "BtoT";
        default:        return Gravity::StringAppender("<unknown_acquire_param:", param, ">").ToString();
    }
}

inline std::string ProbeAckParamToString(int param) noexcept
{
    switch (param)
    {
        case TtoB:      return "TtoB";
        case TtoN:      return "TtoN";
        case BtoN:      return "BtoN";
        case TtoT:      return "TtoT";
        case BtoB:      return "BtoB";
        case NtoN:      return "NtoN";
        default:        return Gravity::StringAppender("<unknown_probeack_param:", param, ">").ToString();
    }
}

inline std::string ReleaseParamToString(int param) noexcept
{
    switch (param)
    {
        case TtoB:      return "TtoB";
        case TtoN:      return "TtoN";
        case BtoN:      return "BtoN";
        case TtoT:      return "TtoT";
        case BtoB:      return "BtoB";
        case NtoN:      return "NtoN";
        default:        return Gravity::StringAppender("<unknown_release_param:", param, ">").ToString();
    }
}

inline std::string PrivilegeToString(int privilege) noexcept
{
    switch (privilege)
    {
        case INVALID:   return "INVALID";
        case BRANCH:    return "BRANCH";
        case TRUNK:     return "TRUNK";
        case TIP:       return "TIP";
        default:        return Gravity::StringAppender("<unknown_privilege:", privilege, ">").ToString();
    }
}

#endif // TLC_TEST_TLENUM_H
