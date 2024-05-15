#pragma once
//
//
//

#ifndef TLC_TEST_PLUGINS_CHISEL_DB
#define TLC_TEST_PLUGINS_CHISEL_DB

#include "PluginManager.hpp"


namespace ChiselDB {

    //
    void InitDB();
    void SaveDB(const char* file);

    //
    class PluginInstance : public Plugin {
    public:
        PluginInstance() noexcept;

    public:
        std::string     GetDisplayName() const noexcept override;
        std::string     GetDescription() const noexcept override;
        std::string     GetVersion() const noexcept override;

        void            OnEnable() override;
        void            OnDisable() override;
    };
    
}


#endif // TLC_TEST_PLUGINS_CHISEL_DB
