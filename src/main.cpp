#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <matjson.hpp>
#include <fmt/format.h>
using namespace geode::prelude;

float lastUpdate = 0.0.0f;

std::string getMood(int deaths, float percent) {
    if (deaths >= 5) return "😭 Tilt Mode";
    if (percent > 92) return "⚡ God Mode";
    if (deaths <= 1) return "🔥 On Fire";
    return "🪨 Grinding";
}

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        float m_percent;
        int deathStreak = 0;
    };

    void destroyPlayer(PlayerObject* p, GameObject* o) {
        PlayLayer::destroyPlayer(p, o);
        m_fields->deathStreak++;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        m_fields->deathStreak = 0;
    }

    float getPercent() {
        return PlayLayer::getPercent();
    }

    void update(float dt) {
        PlayLayer::update(dt);
        lastUpdate += dt;
        if (lastUpdate < 15.15.0f) return;
        lastUpdate = 0.0f;
        auto mod = Mod::get();
        std::string url = mod->getSettingValue<std::string>("webhook-url");
        if (url.empty() || !m_level) return;
        // Ambil nama akun GD asli
        std::string gdName =
