#include "TLSystem.hpp"

#include "../Utils/inicpp.hpp"

#include "../Plugins/ChiselDB.hpp"


void TLInitialize(TLSequencer** tltest, PluginManager** plugins, std::function<void(TLLocalConfig&)> tlcfgInit)
{
    // internal event handlers
    Gravity::RegisterListener(
        Gravity::MakeListener(
            "tltest.logger.plugins.enable", 0,
            (std::function<void(PluginEvent::PostEnable&)>)
            [] (PluginEvent::PostEnable& event) -> void {
                LogInfo("PIM", Append("---PluginManager------------------------------------------------").EndLine());
                LogInfo("PIM", Append("Enabled Plugin:").EndLine());
                LogInfo("PIM", Append(" - name          : ", event.GetPlugin()->GetName()).EndLine());
                LogInfo("PIM", Append(" - display_name  : ", event.GetPlugin()->GetDisplayName()).EndLine());
                LogInfo("PIM", Append(" - version       : ", event.GetPlugin()->GetVersion()).EndLine());
                LogInfo("PIM", Append(" - description   : ", event.GetPlugin()->GetDescription()).EndLine());
                LogInfo("PIM", Append("----------------------------------------------------------------").EndLine());
            }
        )
    );

    Gravity::RegisterListener(
        Gravity::MakeListener(
            "tltest.logger.plugins.disable", 0,
            (std::function<void(PluginEvent::PostDisable&)>)
            [] (PluginEvent::PostDisable& event) -> void {
                LogInfo("PIM", Append("---PluginManager------------------------------------------------").EndLine());
                LogInfo("PIM", Append("Disabled Plugin:").EndLine());
                LogInfo("PIM", Append(" - name          : ", event.GetPlugin()->GetName()).EndLine());
                LogInfo("PIM", Append(" - display_name  : ", event.GetPlugin()->GetDisplayName()).EndLine());
                LogInfo("PIM", Append(" - version       : ", event.GetPlugin()->GetVersion()).EndLine());
                LogInfo("PIM", Append(" - description   : ", event.GetPlugin()->GetDescription()).EndLine());
                LogInfo("PIM", Append("----------------------------------------------------------------").EndLine());
            }
        )
    ); 

    // system initialization
    TLLocalConfig tlcfg;
    tlcfg.seed                          = TLTEST_DEFAULT_SEED;
    tlcfg.coreCount                     = TLTEST_DEFAULT_CORE_COUNT;
    tlcfg.masterCountPerCoreTLC         = TLTEST_DEFAULT_MASTER_COUNT_PER_CORE_TLC;
    tlcfg.masterCountPerCoreTLUL        = TLTEST_DEFAULT_MASTER_COUNT_PER_CORE_TLUL;
    tlcfg.ariInterval                   = CFUZZER_RANGE_ITERATE_INTERVAL;
    tlcfg.ariTarget                     = CFUZZER_RANGE_ITERATE_TARGET;

    tlcfgInit(tlcfg);

    glbl.cfg.verbose                    = false;
    glbl.cfg.verbose_xact_fired         = false;
    glbl.cfg.verbose_xact_sequenced     = false;
    glbl.cfg.verbose_xact_data_complete = false;
    glbl.cfg.verbose_data_full          = false;
    glbl.cfg.verbose_agent_debug        = false;

    // read configuration override
    inicpp::IniManager ini("tltest.ini");

#   define INI_OVERRIDE_INT(section_name, key, target) \
    { \
        auto section = ini[section_name]; \
        if (section.isKeyExist(key)) \
        { \
            target = section.toInt(key); \
            LogInfo("INI", \
                Append("Configuration \'" #target "\' overrided by " section_name ":" key " = ", uint64_t(target), ".").EndLine()); \
        } \
    } \

    INI_OVERRIDE_INT("tltest.logger", "verbose",                    glbl.cfg.verbose);
    INI_OVERRIDE_INT("tltest.logger", "verbose.xact_fired",         glbl.cfg.verbose_xact_fired);
    INI_OVERRIDE_INT("tltest.logger", "verbose.xact_sequenced",     glbl.cfg.verbose_xact_sequenced);
    INI_OVERRIDE_INT("tltest.logger", "verbose.xact_data_complete", glbl.cfg.verbose_xact_data_complete);
    INI_OVERRIDE_INT("tltest.logger", "verbose.data_full",          glbl.cfg.verbose_data_full);
    INI_OVERRIDE_INT("tltest.logger", "verbose.agent_debug",        glbl.cfg.verbose_agent_debug);

    INI_OVERRIDE_INT("tltest.config", "core",                       tlcfg.coreCount);
    INI_OVERRIDE_INT("tltest.config", "core.tl_c",                  tlcfg.masterCountPerCoreTLC);
    INI_OVERRIDE_INT("tltest.config", "core.tl_ul",                 tlcfg.masterCountPerCoreTLUL);

    INI_OVERRIDE_INT("tltest.fuzzer", "seed",                       tlcfg.seed);
    INI_OVERRIDE_INT("tltest.fuzzer", "ari.interval",               tlcfg.ariInterval);
    INI_OVERRIDE_INT("tltest.fuzzer", "ari.target",                 tlcfg.ariTarget);

#   undef INI_OVERRIDE_INT

    //
    (*tltest) = new TLSequencer;
    (*tltest)->Initialize(tlcfg);

    (*plugins) = new PluginManager;
    (*plugins)->EnablePlugin(new ChiselDB::PluginInstance);
}

void TLFinalize(TLSequencer** tltest, PluginManager** plugins)
{
    (*plugins)->DisableAll([](auto plugin) -> void { delete plugin; });
    delete *plugins;
    
    (*tltest)->Finalize();
    delete *tltest;
}
