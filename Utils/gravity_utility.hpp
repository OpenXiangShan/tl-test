#pragma once

#ifndef __BULLSEYE_SIMS_GRAVITY__UTILITY
#define __BULLSEYE_SIMS_GRAVITY__UTILITY

#include <sstream>
#include <iomanip>


namespace Gravity {

    /*
    * String Appender (builder pattern helper for std::ostringstream)
    * ----------------------------------------------------------------
    * Usage:
    *   StringAppender appender("Hello");
    *   appender.Append(", ").Append("World!").Append(123).ToString();
    * or
    *   StringAppender().Append("Hello, ").Append("World!").Append(123).ToString();
    *   StringAppender("Hello, ").Append("World!").Append(123).ToString();
    */
    class StringAppender {
    private:
        std::ostringstream  oss;

    public:
        inline StringAppender() noexcept {};
        inline ~StringAppender() noexcept {};

        template<class T>
        inline StringAppender(const T& value) noexcept 
        { oss << value; }

        template<class T, class... U>
        inline StringAppender(const T& value, const U&... args) noexcept 
        { oss << value; Append(args...); }

        inline StringAppender& Hex() noexcept
        { oss << std::hex; return *this; }

        inline StringAppender& Dec() noexcept
        { oss << std::dec; return *this; }

        inline StringAppender& Oct() noexcept
        { oss << std::oct; return *this; }

        inline StringAppender& Fixed() noexcept
        { oss << std::fixed; return *this; }

        inline StringAppender& Scientific() noexcept
        { oss << std::scientific; return *this; }
        
        inline StringAppender& HexFloat() noexcept
        { oss << std::hexfloat; return *this; }

        inline StringAppender& DefaultFloat() noexcept
        { oss << std::defaultfloat; return *this; }

        inline StringAppender& Base(int n) noexcept
        { oss << std::setbase(n); return *this; }

        template<class CharT>
        inline StringAppender& Fill(CharT c) noexcept
        { oss << std::setfill(c); return *this; }

        inline StringAppender& Precision(int n) noexcept
        { oss << std::setprecision(n); return *this; }

        inline StringAppender& NextWidth(int n) noexcept
        { oss << std::setw(n); return *this; }

        inline StringAppender& BoolAlpha() noexcept
        { oss << std::boolalpha; return *this; }

        inline StringAppender& NoBoolAlpha() noexcept
        { oss << std::noboolalpha; return *this; }

        inline StringAppender& ShowBase() noexcept
        { oss << std::showbase; return *this; }

        inline StringAppender& NoShowBase() noexcept
        { oss << std::noshowbase; return *this; }

        inline StringAppender& ShowPoint() noexcept
        { oss << std::showpoint; return *this; }

        inline StringAppender& NoShowPoint() noexcept
        { oss << std::noshowpoint; return *this; }

        inline StringAppender& ShowPos() noexcept
        { oss << std::showpos; return *this; }

        inline StringAppender& NoShowPos() noexcept
        { oss << std::noshowpos; return *this; }

        inline StringAppender& SkipWs() noexcept
        { oss << std::skipws; return *this; }

        inline StringAppender& NoSkipWs() noexcept
        { oss << std::noskipws; return *this; }

        inline StringAppender& Left() noexcept
        { oss << std::left; return *this; }

        inline StringAppender& Right() noexcept
        { oss << std::right; return *this; }

        inline StringAppender& Internal() noexcept
        { oss << std::internal; return *this; }

        inline StringAppender& NewLine() noexcept
        { oss << std::endl; return *this; }

        inline StringAppender& EndLine() noexcept
        { oss << std::endl; return *this; }

        inline StringAppender& Append() noexcept 
        { return *this; }

        template<class T>
        inline StringAppender& Append(const T& value) noexcept 
        { oss << value; return *this; }

        template<class T, class... U>
        inline StringAppender& Append(const T& value, const U&... args) noexcept 
        { oss << value; return Append(args...); }

        template<class T>
        inline StringAppender& operator<<(const T& value) noexcept 
        { oss << value; return *this; }

        inline std::string ToString() const noexcept 
        { return oss.str(); }
    };
}


#endif // __BULLSEYE_SIMS_GRAVITY__UTILITY
