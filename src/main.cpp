#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/binding/GJAccountManager.hpp> // Ambil nama user GD
#include <ctime>
#include <vector>
#include <string>

using namespace geode::prelude;

// Quotes untuk recap
std::vector<std::string> getQuotes(bool roast) {
    if (roast) {
        return {"Skill issue lagi bro 😂", "Mati terus? Classic GD noob", "Another one bites the dust 💀", "Try harder next time kiddo", "GD players when they see a spike: 😭"};
    } else {
        return {"Keep grinding king! 🔥", "Progress is progress, mantap!", "You're getting better every day!", "Streak on fire! Jangan berhenti", "Legendary effort, keep it up!"};
    }
}

// Fungsi kirim recap ke Discord
void sendRecap() {
    std::string url = Mod::get()->getSettingValue<std::string>("webhook-url");
    if (url.empty()) return;

    // Ambil nama GD player otomatis
    std::string name = "Player";
    if (GJAccountManager::sharedState() && !GJAccountManager::sharedState()->m_username.empty())
        name = GJAccountManager::sharedState()->m_username;

    bool roast = Mod::get()->getSettingValue<bool>("roast-mode");

    int deaths = Mod::get()->getSavedValue<int>("totalDeaths", 0);
    int levels = Mod::get()->getSavedValue<int>("totalLevels", 0);
    int stars = Mod::get()->getSavedValue<int>("totalStars", 0);
    int moons = Mod::get()->getSavedValue<int>("totalMoons", 0); // Pastikan moons direkam
    int streak = Mod::get()->getSavedValue<int>("streak", 0);

    auto quotes = getQuotes(roast);
    srand(static_cast<unsigned>(time(nullptr)));
    std::string quote = quotes[rand() % quotes.size()];

    // Format pesan dengan nama, stars, moons
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

    auto json = matjson::Value();
    json["content"] = msg;
    web::WebRequest().post(url).json(json).fetch();
}

// Logika trigger recap (mis: harian/weekly)
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

// Contoh modifikasi event PlayLayer untuk update progres
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
            int moonsEarned = m_level->m_moons; // jika ada, pastikan binding moons (GD 2.2+)
            int levels = Mod::get()->getSavedValue<int>("totalLevels", 0) + 1;
            int stars = Mod::get()->getSavedValue<int>("totalStars", 0) + starsEarned;
            int moons = Mod::get()->getSavedValue<int>("totalMoons", 0) + moonsEarned;
            Mod::get()->setSavedValue("totalLevels", levels);
            Mod::get()->setSavedValue("totalStars", stars);
            Mod::get()->setSavedValue("totalMoons", moons);
            checkAndSend();
        }
    }
};