#pragma once

#include <exception>
#ifndef TLCTEST_EVENTS_ASSERT_HEADER
#define TLCTEST_EVENTS_ASSERT_HEADER

#include <string>

#include "../Utils/gravity_eventbus.hpp"


class TLAssertFailureEvent : public Gravity::Event<TLAssertFailureEvent> {
private:
    std::string     sourceLocation;
    std::string     info;

public:
    TLAssertFailureEvent() noexcept;
    TLAssertFailureEvent(const std::string& sourceLocation, const std::string& info) noexcept;

    const std::string&  GetSourceLocation() const noexcept;
    void                SetSourceLocation(const std::string& info) noexcept;

    const std::string&  GetInfo() const noexcept;
    void                SetInfo(const std::string& info) noexcept;
};


class TLAssertFailureException : public std::exception {
private:
    TLAssertFailureEvent    copiedEvent;

public:
    TLAssertFailureException(const TLAssertFailureEvent& event) noexcept;

    const TLAssertFailureEvent&     GetCopiedEvent() const noexcept;
};



// Implementation of: class TLAssertFailureEvent
/*
std::string     sourceLocation;
std::string     info;
*/

inline TLAssertFailureEvent::TLAssertFailureEvent() noexcept
    : sourceLocation    ()
    , info              ()
{ }

inline TLAssertFailureEvent::TLAssertFailureEvent(
        const std::string& sourceLocation, 
        const std::string& info) noexcept
    : sourceLocation    (sourceLocation)
    , info              (info)
{ }

inline const std::string& TLAssertFailureEvent::GetSourceLocation() const noexcept
{
    return sourceLocation;
}

inline void TLAssertFailureEvent::SetSourceLocation(const std::string& sourceLocation) noexcept
{
    this->sourceLocation = sourceLocation;
}

inline const std::string& TLAssertFailureEvent::GetInfo() const noexcept
{
    return info;
}

inline void TLAssertFailureEvent::SetInfo(const std::string& info) noexcept
{
    this->info = info;
}


// Implementation of: class TLAssertFailureException
/*
TLAssertFailureEvent    copiedEvent;
*/

inline TLAssertFailureException::TLAssertFailureException(const TLAssertFailureEvent& event) noexcept
    : copiedEvent   (event)
{ }

inline const TLAssertFailureEvent& TLAssertFailureException::GetCopiedEvent() const noexcept
{
    return copiedEvent;
}


#endif // TLCTEST_EVENTS_ASSERT_HEADER
