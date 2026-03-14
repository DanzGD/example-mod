#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <ctime>
#include <vector>
#include <cstdlib>

using namespace geode::prelude;

std::vector<std::string> getQuotes(bool roast) {
    if (roast) {
        return {"Skill issue lagi bro 😂", "Mati terus? Classic GD noob", "Another one bites the dust 💀", "Try harder next time kiddo", "GD players when they see a spike: 😭"};
    } else {
        return {"Keep grinding king! 🔥", "Progress is progress, mantap!", "You're getting better every day!", "Streak on fire! Jangan berhenti", "Legendary effort, keep it up!"};
    }
}

void sendRecap() {
    std::string url = Mod::get()->getSettingValue<std::string>("webhook-url");
    if (url.empty()) return;

    std::string name = Mod::get()->getSettingValue<std::string>("custom-name");
    bool roast = Mod::get()->getSettingValue<bool>("roast-mode");

    int deaths = Mod::get()->getSavedValue<int>("totalDeaths", 0);
    int levels = Mod::get()->getSavedValue<int>("totalLevels", 0);
    int stars = Mod::get()->getSavedValue<int>("totalStars", 0);
    int streak = Mod::get()->getSavedValue<int>("streak", 0);

    auto quotes = getQuotes(roast);
    srand(static_cast<unsigned>(time(nullptr)));
    std::string quote = quotes[rand() % quotes.size()];

    std::string msg = fmt::format(
        "**🦾 GD Progress Bot Recap**\n\n"
        "👤 **{}**\n"
        "⭐ **Total Stars**: {}\n"
        "💀 **Total Deaths**: {}\n"
        "🏆 **Levels Beaten**: {}\n"
        "🔥 **Streak**: {} hari berturut-turut\n\n"
        "💬 {}\n\nKeep pushing! 💪",
        name, stars, deaths, levels, streak, quote
    );

    auto json = matjson::Value();
    json["content"] = msg;
    web::WebRequest().post(url).json(json).fetch();
}

void checkAndSend() {
    std::string freq = Mod::get()->getSettingValue<std::string>("frequency");
    if (freq != "daily" && freq != "weekly") return;

    long long now = time(nullptr) / 86400LL;
    long long lastSent = Mod::get()->getSavedValue<long long>("lastSentDay", 0);

    bool should = false;
    if (freq == "daily" && now > lastSent) should = true;
    else if (freq == "weekly" && (now / 7) > (lastSent / 7)) should = true;

    if (should) {
        sendRecap();
        Mod::get()->setSavedValue("lastSentDay", now);
    }
}

class $modify(PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        int deaths = Mod::get()->getSavedValue<int>("totalDeaths", 0) + 1;
        Mod::get()->setSavedValue("totalDeaths", deaths);
        checkAndSend();
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        if (m_level) {
            int starsEarned = m_level->m_stars;
            int levels = Mod::get()->getSavedValue<int>("totalLevels", 0) + 1;
            int totalStars = Mod::get()->getSavedValue<int>("totalStars", 0) + starsEarned;
            Mod::get()->setSavedValue("totalLevels", levels);
            Mod::get()->setSavedValue("totalStars", totalStars);

            long long now = time(nullptr) / 86400LL;
            long long lastDay = Mod::get()->getSavedValue<long long>("lastActiveDay", 0);
            int streak = Mod::get()->getSavedValue<int>("streak", 0);
            if (now == lastDay + 1) streak++;
            else if (now > lastDay + 1) streak = 1;
            Mod::get()->setSavedValue("streak", streak);
            Mod::get()->setSavedValue("lastActiveDay", now);
        }
        checkAndSend();
    }
};

$execute {
    srand(static_cast<unsigned>(time(nullptr)));
}