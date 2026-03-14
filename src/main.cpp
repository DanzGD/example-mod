#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>

using namespace geode::prelude;

std::vector<std::string> getQuotes(bool roast) {
    if (roast) return {"Skill issue lagi bro 😂", "Mati terus? Classic GD noob", "Another one bites the dust 💀", "Try harder next time kiddo", "GD players when they see a spike: 😭"};
    return {"Keep grinding king! 🔥", "Progress is progress, mantap!", "You're getting better every day!", "Streak on fire! Jangan berhenti", "Legendary effort, keep it up!"};
}

void sendRecap() {
    auto mod = Mod::get();
    std::string url = mod->getSettingValue<std::string>("webhook-url");
    if (url.empty()) return;

    std::string name = "Player";
    if (auto acc = GJAccountManager::sharedState()) {
        if (!acc->m_username.empty()) name = acc->m_username;
    }

    bool roast = mod->getSettingValue<bool>("roast-mode");

    int deaths = mod->getSavedValue<int>("totalDeaths", 0);
    int levels = mod->getSavedValue<int>("totalLevels", 0);
    int stars  = mod->getSavedValue<int>("totalStars", 0);
    int moons  = mod->getSavedValue<int>("totalMoons", 0);
    int streak = mod->getSavedValue<int>("streak", 0);

    auto quotes = getQuotes(roast);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    std::string quote = quotes[std::rand() % quotes.size()];

    std::string msg = fmt::format(
        "**🦾 GD Progress Bot Recap**\n\n"
        "👤 **{}**\n"
        "⭐ **Total Stars**: {}\n"
        "🌙 **Total Moons**: {}\n"
        "💀 **Total Deaths**: {}\n"
        "🏆 **Levels Beaten**: {}\n"
        "🔥 **Streak**: {} hari\n\n"
        "💬 {}\n\nKeep pushing! 💪",
        name, stars, moons, deaths, levels, streak, quote
    );

    matjson::Value json = matjson::Value::object();
    json["content"] = msg;

    web::WebRequest req;
    req.bodyJSON(json);
    req.header("Content-Type", "application/json");
    req.post(url);   // fire and forget — sesuai docs Geode
}

void checkAndSend() {
    auto mod = Mod::get();
    std::string freq = mod->getSettingValue<std::string>("frequency");
    if (freq != "daily" && freq != "weekly") return;

    auto now = static_cast<long long>(std::time(nullptr)) / 86400LL;
    auto last = mod->getSavedValue<long long>("lastSentDay", 0LL);

    bool should = false;
    if (freq == "daily" && now > last) should = true;
    else if (freq == "weekly" && (now / 7) > (last / 7)) should = true;

    if (should) {
        sendRecap();
        mod->setSavedValue("lastSentDay", now);
        mod->setSavedValue("streak", mod->getSavedValue<int>("streak", 0) + 1);
    }
}

$modify(PlayLayer) {
    void destroyPlayer(PlayerObject* p, GameObject* o) {
        PlayLayer::destroyPlayer(p, o);
        Mod::get()->setSavedValue("totalDeaths", Mod::get()->getSavedValue<int>("totalDeaths", 0) + 1);
    }
};

$modify(EndLevelLayer) {
    bool init(PlayLayer* playLayer) {
        if (!EndLevelLayer::init(playLayer)) return false;

        if (playLayer && playLayer->m_level && this->m_levelComplete) {
            auto lvl = playLayer->m_level;

            Mod::get()->setSavedValue("totalLevels", Mod::get()->getSavedValue<int>("totalLevels", 0) + 1);
            Mod::get()->setSavedValue("totalStars", Mod::get()->getSavedValue<int>("totalStars", 0) + lvl->m_stars);

            checkAndSend();
        }
        return true;
    }
};