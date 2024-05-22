#pragma once

#ifndef TLC_TEST_STARTUP_H
#define TLC_TEST_STARTUP_H

#include "../Sequencer/TLSequencer.hpp"
#include "../Plugins/PluginManager.hpp"

#include <functional>


void TLInitialize(TLSequencer** tltest, PluginManager** plugins, std::function<void(TLLocalConfig&)> tlcfgInit);
void TLFinalize(TLSequencer** tltest, PluginManager** plugins);


#endif // TLC_TEST_STARTUP_H

