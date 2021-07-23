add_subdirectory(Processors)
add_subdirectory(Presets)

target_sources(CutTheMoog PRIVATE
        PluginProcessor.cpp
        PowerButton.cpp
)