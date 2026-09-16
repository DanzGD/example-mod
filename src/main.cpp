#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/utils/web.hpp>
#include <matjson.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class DanzGDPSPopup : public FLAlertLayer {
protected:
    bool init() {
        if (!FLAlertLayer::init(310.f, 220.f, "DanzGDPS Legacy", "Close", nullptr, 1.f))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto lotm = CCLabelBMFont::create("LOTM", "bigFont.fnt");
        lotm->setScale(0.65f);
        lotm->setPosition({155.f, 175.f});
        m_mainLayer->addChild(lotm);

        auto lotmInfo = CCLabelBMFont::create("Loading Level of the Month...", "goldFont.fnt");
        lotmInfo->setScale(0.42f);
        lotmInfo->setPosition({155.f, 145.f});
        m_mainLayer->addChild(lotmInfo);

        auto announcements = CCLabelBMFont::create("ANNOUNCEMENTS", "bigFont.fnt");
        announcements->setScale(0.55f);
        announcements->setPosition({155.f, 105.f});
        m_mainLayer->addChild(announcements);

        auto news = CCLabelBMFont::create("Loading announcements...", "goldFont.fnt");
        news->setScale(0.38f);
        news->setPosition({155.f, 75.f});
        m_mainLayer->addChild(news);

        // The API URL can be changed later without changing the UI code.
        auto mod = Mod::get();
        auto lotmURL = mod->getSettingValue<std::string>("lotm-api");
        auto announcementsURL = mod->getSettingValue<std::string>("announcements-api");

        if (!lotmURL.empty()) {
            web::WebRequest().get(lotmURL).listen([lotmInfo](web::WebResponse res) {
                if (!res.ok()) return;
                auto json = res.json();
                if (!json) return;
                auto name = json.unwrap().get<std::string>("name").unwrapOr("Unknown Level");
                auto creator = json.unwrap().get<std::string>("creator").unwrapOr("Unknown Creator");
                lotmInfo->setString(fmt::format("{} by {}", name, creator).c_str());
            });
        }

        if (!announcementsURL.empty()) {
            web::WebRequest().get(announcementsURL).listen([news](web::WebResponse res) {
                if (!res.ok()) return;
                auto json = res.json();
                if (!json) return;
                auto value = json.unwrap();
                auto title = value.get<std::string>("title").unwrapOr("No announcements");
                news->setString(title.c_str());
            });
        }

        return true;
    }

public:
    static DanzGDPSPopup* create() {
        auto ret = new DanzGDPSPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class $modify(DanzMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu, 100);

        auto buttonSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        if (!buttonSprite) {
            buttonSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        }
        buttonSprite->setScale(0.8f);

        auto button = CCMenuItemSpriteExtra::create(
            buttonSprite,
            this,
            menu_selector(DanzMenuLayer::onDanzGDPS)
        );
        button->setID("danzgdps-legacy-button");
        menu->addChild(button);
        menu->setPosition({25.f, 25.f});
        return true;
    }

    void onDanzGDPS(CCObject*) {
        DanzGDPSPopup::create()->show();
    }
};
