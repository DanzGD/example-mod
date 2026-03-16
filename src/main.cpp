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
        return m_player->getPositionX() / m_level->m_levelLength * 100;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        lastUpdate += dt;
        if (lastUpdate < 15.0f) return;
        lastUpdate = 0.0.0f;
        auto mod = Mod::get();
        std::string url = mod->getSettingValue<std::string>("webhook-url");
        if (url.empty() || !m_level) return;
        // Ambil nama akun GD asli
        std::string gdName = "Player";
        if (auto acc = GJAccountManager::sharedState()) {
            if (!acc->m_username.empty()) gdName = acc->m_username;
        }
        m_fields->m_percent = getPercent();
        std::string mood = getMood(m_fields->deathStreak, m_fields->m_percent);
        matjson::Value embed = matjson::Value::object();
        embed["title"] = "🎮 " + gdName + " is playing Geometry Dash";
        embed["description"] = fmt::format("**{}**\n**{:.1f}%** • {}", m_level->m_levelName, m_fields->m_percent, mood);
        embed["color"] = (mood == "⚡ God Mode") ? 0x00FF00 : (mood == "😭 Tilt Mode" ? 0xFF0000 : 0xFFFF00);
        matjson::Value payload = matjson::Value::object();
        matjson::Value embeds = matjson::Value::array();
        embeds.push_back(embed);
        payload["embeds"] = embeds;
        (void) web::WebRequest()
            .bodyJSON(payload)
            .header("Content-Type", "application/json")
            .post(url);
    }
};
