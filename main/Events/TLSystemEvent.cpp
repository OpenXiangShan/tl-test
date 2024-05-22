#include "TLSystemEvent.hpp"
//
// Created by Kumonda221 on 2024-05-06
//


// Implementation of: class TLSystemInitializationPreEvent
TLSystemInitializationPreEvent::TLSystemInitializationPreEvent(TLLocalConfig* config) noexcept
    : config    (config)
{ }

// Implementation of: class TLSystemInitializationPostEvent
TLSystemInitializationPostEvent::TLSystemInitializationPostEvent(TLSequencer* system) noexcept
    : system    (system)
{ }



// Implementation of: class TLSystemFinalizationPreEvent
TLSystemFinalizationPreEvent::TLSystemFinalizationPreEvent(TLSequencer* system) noexcept
    : system    (system)
{ }

// Implementation of: class TLSystemFinalizationPostEvent

