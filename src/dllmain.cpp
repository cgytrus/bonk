#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <matdash.hpp>
#include <matdash/minhook.hpp>
#include <matdash/boilerplate.hpp>
//#include <matdash/console.hpp>
#pragma warning(push, 0)
#include <cocos2d.h>
#pragma warning(pop)
#include <gd.h>

#include "config.hpp"

using namespace cocos2d;

const std::string VanillaSound = "explode_11.ogg";
const std::string BonkSound = "bonk.ogg";
const double NonDeathBonkThreshold = 4.0;

static gd::GameObject* _deathCause = nullptr;
static std::string _customDeathSound = VanillaSound;

std::unordered_map<gd::PlayerObject*, double> _prevYAccel;
std::unordered_map<gd::PlayerObject*, uint8_t> _prevIgnoreFlags;

// it has some 3 float args but i just pretend they dont exist cuz theyre in xmm registers
matdash::cc::stdcall<void> GameSoundManager_playEffect(std::string file) {
    matdash::orig<&GameSoundManager_playEffect>(file == VanillaSound ? _customDeathSound : file);
    return {};
}

// same here with the xmm registers, both of these have a float in xmm1 as the first argument
matdash::cc::thiscall<bool> PlayerObject_collidedWithObject(gd::PlayerObject* self, gd::GameObject* obj, CCRect idk) {
    _deathCause = obj;
    return matdash::orig<&PlayerObject_collidedWithObject>(self, obj, idk);
}
matdash::cc::thiscall<void> PlayerObject_collidedWithSlope(gd::PlayerObject* self, gd::GameObject* obj, bool idk) {
    _deathCause = obj;
    matdash::orig<&PlayerObject_collidedWithSlope>(self, obj, idk);
    return {};
}

bool isRaising(gd::PlayerObject* player) {
    return player->m_isUpsideDown ? player->m_yAccel < 0.0 : player->m_yAccel > 0.0;
}
bool isBonkableObject(gd::GameObject* obj) {
    return obj->m_nObjectType == gd::GameObjectType::kGameObjectTypeSolid ||
        obj->m_nObjectType == gd::GameObjectType::kGameObjectTypeSlope;
}
matdash::cc::thiscall<void> PlayLayer_destroyPlayer(gd::PlayLayer* self, gd::PlayerObject* ply, gd::GameObject* obj) {
    if(obj != nullptr)
        _deathCause = obj;
    // if we hit a solid or a slope with an upwards velocity, play the bonk sound instead
    _customDeathSound = _deathCause && (config::onWaveDeath && ply->m_isDart || !ply->m_isDart && isRaising(ply)) &&
        isBonkableObject(_deathCause) ? BonkSound : VanillaSound;
    _prevYAccel.erase(ply);
    _prevIgnoreFlags.erase(ply);
    matdash::orig<&PlayLayer_destroyPlayer>(self, ply, obj);
    return {};
}

uint8_t getIgnoreFlags(gd::PlayerObject* player) {
    return static_cast<uint8_t>(player->m_isUpsideDown) |
        static_cast<uint8_t>(player->m_isShip << 1) |
        static_cast<uint8_t>(player->m_isBird << 2) |
        static_cast<uint8_t>(player->m_isBall << 3) |
        static_cast<uint8_t>(player->m_isDart << 4) |
        static_cast<uint8_t>(player->m_isRobot << 5) |
        static_cast<uint8_t>(player->m_isSpider << 6);
}

matdash::cc::thiscall<void> PlayerObject_dector(gd::PlayerObject* self) {
    _prevYAccel.erase(self);
    _prevIgnoreFlags.erase(self);
    matdash::orig<&PlayerObject_dector>(self);
    return {};
}
bool hasHitHead(gd::PlayerObject* player, double yAccel) {
    return yAccel == 0.0 ? _prevYAccel[player] >= NonDeathBonkThreshold :
        std::abs(_prevYAccel[player] - yAccel) >= NonDeathBonkThreshold && yAccel < 0.0;
}
bool waveHitSomething(gd::PlayerObject* player, double yAccel) {
    return std::abs(_prevYAccel[player] - yAccel) >= NonDeathBonkThreshold &&
        (player->m_isHolding ? yAccel <= 0.0 : yAccel >= 0.0);
}
matdash::cc::thiscall<void> PlayLayer_checkCollisions(gd::PlayLayer* self, gd::PlayerObject* player) {
    matdash::orig<&PlayLayer_checkCollisions>(self, player);
    if(config::onlyOnDeath || player->m_isDead)
        return {};
    double yAccel = player->m_isUpsideDown ? -player->m_yAccel : player->m_yAccel;
    uint8_t ignoreFlags = getIgnoreFlags(player);
    if(_prevYAccel.find(player) != _prevYAccel.end() && _prevIgnoreFlags[player] == ignoreFlags &&
        (player->m_isDart ? config::onWaveSlide && waveHitSomething(player, yAccel) : hasHitHead(player, yAccel)))
        matdash::orig<&GameSoundManager_playEffect>(BonkSound);
    _prevYAccel[player] = yAccel;
    _prevIgnoreFlags[player] = ignoreFlags;
    return {};
}

void addHooks() {
    matdash::add_hook<&GameSoundManager_playEffect>(gd::base + 0x25450);
    matdash::add_hook<&PlayerObject_collidedWithObject>(gd::base + 0x1ebdd0);
    matdash::add_hook<&PlayerObject_collidedWithSlope>(gd::base + 0x1eace0);
    matdash::add_hook<&PlayLayer_destroyPlayer>(gd::base + 0x20a1a0);

    matdash::add_hook<&PlayerObject_dector>(gd::base + 0x1e6be0);
    matdash::add_hook<&PlayLayer_checkCollisions>(gd::base + 0x203cd0);
}

void mod_main(HMODULE hModule) {
    //matdash::create_console();
    config::loadConfig();
    config::initMegahack();
    if(config::enabled)
        config::enableHooks();
}
