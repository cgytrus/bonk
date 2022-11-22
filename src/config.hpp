#pragma once

#pragma warning(push, 0)
#include <cocos2d.h>
#pragma warning(pop)
#include <filesystem>
#include <fstream>
#include "extensions2.h"

namespace bonk {
    void addHooks();
}

namespace bonk::config {
    static bool enabled = true;
    static bool onlyOnDeath = true;
    static bool onWaveDeath = true;
    static bool onWaveSlide = false;

    std::filesystem::path getConfigPath() {
        auto configDir = std::filesystem::path(cocos2d::CCFileUtils::sharedFileUtils()->getWritablePath2()) / "config";
        std::filesystem::create_directories(configDir);
        return configDir / "bonk.txt";
    }
    void loadConfig() {
        auto configPath = getConfigPath();
        if(!std::filesystem::exists(configPath))
            return;
        std::ifstream config(configPath);
        config >> enabled >> onlyOnDeath >> onWaveDeath >> onWaveSlide;
        config.close();
    }
    void saveConfig() {
        std::ofstream config(getConfigPath());
        config << enabled << ' ' << onlyOnDeath << ' ' << onWaveDeath << ' ' << onWaveSlide;
        config.close();
    }

    static bool _wasEverEnabled = false;
    void enableHooks() {
        if(_wasEverEnabled) {
            MH_EnableHook(MH_ALL_HOOKS);
            return;
        }
        _wasEverEnabled = true;
        addHooks();
    }
    void disableHooks() {
        if(_wasEverEnabled)
            MH_DisableHook(MH_ALL_HOOKS);
    }

    void initMegahack() {
        if(!GetModuleHandle(TEXT("hackpro.dll")))
            return;

        auto window = MegaHackExt::Window::Create("bonk");

        auto checkbox = MegaHackExt::CheckBox::Create("Bonk on wave slide");
        checkbox->set(onWaveSlide);
        checkbox->setCallback([](MegaHackExt::CheckBox* obj, bool value) {
            onWaveSlide = value;
            saveConfig();
        });
        window->add(checkbox);

        checkbox = MegaHackExt::CheckBox::Create("Bonk on wave death");
        checkbox->set(onWaveDeath);
        checkbox->setCallback([](MegaHackExt::CheckBox* obj, bool value) {
            onWaveDeath = value;
            saveConfig();
        });
        window->add(checkbox);

        checkbox = MegaHackExt::CheckBox::Create("Bonk only on death");
        checkbox->set(onlyOnDeath);
        checkbox->setCallback([](MegaHackExt::CheckBox* obj, bool value) {
            onlyOnDeath = value;
            saveConfig();
        });
        window->add(checkbox);

        checkbox = MegaHackExt::CheckBox::Create("Enabled");
        checkbox->set(enabled);
        checkbox->setCallback([](MegaHackExt::CheckBox* obj, bool value) {
            enabled = value;
            if(value)
                enableHooks();
            else
                disableHooks();
            saveConfig();
        });
        window->add(checkbox);

        MegaHackExt::Client::commit(window);
    }
}
