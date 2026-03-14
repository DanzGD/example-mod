#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <matjson.hpp>
#include <fmt/format.h>
#include <ctime>
#include <cstdlib>

using namespace geode::prelude;

std::vector<std::string> getQuotes(bool roast) {
    if (roast) {
        return {
            "Skill issue lagi bro 😂",
            "Mati terus? Classic GD noob",
            "Another one bites the dust 💀",
            "Try harder next time kiddo",
            "GD players when they see a spike: 😭"
        };
    }
    return {
        "Keep grinding king! 🔥",
        "Progress is progress, mantap!",
        "You're getting better every day!",
        "Streak on fire! Jangan berhenti",
        "Legendary effort, keep it up!"
    };
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

    int deaths  = mod->getSavedValue<int>("totalDeaths", 0);
    int levels  = mod->getSavedValue<int>("totalLevels", 0);
    int stars   = mod->getSavedValue<int>("totalStars", 0);
    int moons   = mod->getSavedValue<int>("totalMoons", 0);
    int streak  = mod->getSavedValue<int>("streak", 0);

    auto quotes = getQuotes(roast);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    std::string quote = quotes[rand() % quotes.size()];

    std::string msg = fmt::format(
        "**🦾 GD Progress Bot Recap**\n\n"
        "👤 **{}**\n"
        "⭐ **Total Stars**: {}\n"
        "🌙 **Total Moons**: {}\n"
        "💀 **Total Deaths**: {}\n"
        "🏆 **Levels Beaten**: {}\n"
        "🔥 **Streak**: {} hari berturut-turut\n\n"
        "💬 {}\n\nKeep pushing! 💪",
        name, stars, moons, deaths, levels, streak, quote
    );

    matjson::Value json;
    json["content"] = msg;

    // Kirim ke Discord (async)
    web::WebRequest().post(url, json.dump())
        .header("Content-Type", "application/json")
        .fetch();

    log::info("✅ Recap dikirim ke Discord!");
}

void checkAndSend() {
    auto mod = Mod::get();
    std::string freq = mod->getSettingValue<std::string>("frequency");
    if (freq != "daily" && freq != "weekly") return;

    long long now = static_cast<long long>(std::time(nullptr)) / 86400LL;
    long long last = mod->getSavedValue<long long>("lastSentDay", 0);

    bool shouldSend = false;
    if (freq == "daily" && now > last) shouldSend = true;
    else if (freq == "weekly" && (now / 7) > (last / 7)) shouldSend = true;

    if (shouldSend) {
        sendRecap();
        mod->setSavedValue("lastSentDay", now);

        // Update streak (hanya +1 tiap recap berhasil)
        int streak = mod->getSavedValue<int>("streak", 0) + 1;
        mod->setSavedValue("streak", streak);
    }
}

// Hook kematian (untuk hitung deaths)
class $modify(PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        int deaths = Mod::get()->getSavedValue<int>("totalDeaths", 0) + 1;
        Mod::get()->setSavedValue("totalDeaths", deaths);
    }
};

// Hook utama: saat End Level Screen muncul (lebih stabil daripada levelComplete)
class $modify(EndLevelLayer) {
    bool init(PlayLayer* playLayer) {
        if (!EndLevelLayer::init(playLayer)) return false;

        if (playLayer && playLayer->m_level) {
            auto lvl = playLayer->m_level;

            int starsEarned = lvl->m_stars;
            int moonsEarned = 0; // 2.2 platformer moons (bisa di-upgrade nanti)

            int levels = Mod::get()->getSavedValue<int>("totalLevels", 0) + 1;
            int stars  = Mod::get()->getSavedValue<int>("totalStars", 0) + starsEarned;
            int moons  = Mod::get()->getSavedValue<int>("totalMoons", 0) + moonsEarned;

            Mod::get()->setSavedValue("totalLevels", levels);
            Mod::get()->setSavedValue("totalStars", stars);
            Mod::get()->setSavedValue("totalMoons", moons);

            checkAndSend();
        }
        return true;
    }
};