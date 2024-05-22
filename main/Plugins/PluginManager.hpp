#pragma once
//
//
//

#ifndef TLC_TEST_PLUGIN_MANAGER
#define TLC_TEST_PLUGIN_MANAGER

#include <string>
#include <map>
#include <functional>

#include "../Utils/gravity_eventbus.hpp"


class Plugin {
private:
    const std::string       name;

public:
    Plugin(const char* name) noexcept;
    Plugin(const std::string& name) noexcept;
    virtual ~Plugin();

public:
    virtual std::string GetName() const noexcept final;
    virtual std::string GetDisplayName() const noexcept = 0;
    virtual std::string GetVersion() const noexcept = 0;
    virtual std::string GetDescription() const noexcept = 0;

public:
    virtual void        OnEnable() = 0;
    virtual void        OnDisable() = 0;
};


class PluginManager {
private:
    std::map<std::string, Plugin*>  plugins;

public:
    using iterator              = std::map<std::string, Plugin*>::iterator;
    using const_iterator        = std::map<std::string, Plugin*>::const_iterator;

public:
    void                        EnablePlugin(Plugin* plugin);
    Plugin*                     DisablePlugin(const std::string& name);

    void                        DisableAll(std::function<void(Plugin*)> callback);

    Plugin*                     GetPlugin(const std::string& name) noexcept;
    const Plugin*               GetPlugin(const std::string& name) const noexcept;
    bool                        HasPlugin(const std::string& name) const noexcept;

    iterator                    begin() noexcept;
    const_iterator              begin() const noexcept;

    iterator                    end() noexcept;
    const_iterator              end() const noexcept;
};


namespace PluginEvent {

    //
    class Base {
    private:
        PluginManager*  const manager;
        Plugin*         const plugin;

    public:
        Base(PluginManager* manager, Plugin* plugin) noexcept;

    public:
        PluginManager*  GetManager() const noexcept;
        Plugin*         GetPlugin() const noexcept;
    };

    //
    class PreEnable : public Base, public Gravity::Event<PreEnable> 
    { 
    public:
        PreEnable(PluginManager* manager, Plugin* plugin) noexcept;
    };

    class PostEnable : public Base, public Gravity::Event<PostEnable> 
    { 
    public:
        PostEnable(PluginManager* manager, Plugin* plugin) noexcept;
    };

    //
    class PreDisable : public Base, public Gravity::Event<PreDisable> 
    { 
    public:
        PreDisable(PluginManager* manager, Plugin* plugin) noexcept;
    };

    class PostDisable : public Base, public Gravity::Event<PostDisable> 
    { 
    public:
        PostDisable(PluginManager* manager, Plugin* plugin) noexcept;
    };
}


#endif // TLC_TEST_PLUGIN_MANAGER
