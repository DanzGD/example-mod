#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/utils/web.hpp>
#include <matjson.hpp>

using namespace geode::prelude;

class DanzGDPSPopup : public FLAlertLayer {
protected:
    bool init() {
        if (!FLAlertLayer::init(310.f, 220.f, "DanzGDPS Legacy", "Close", nullptr, 1.f))
            return false;

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

        auto mod = Mod::get();
        auto lotmURL = mod->getSettingValue<std::string>("lotm-api");
        auto announcementsURL = mod->getSettingValue<std::string>("announcements-api");

        if (!lotmURL.empty()) {
            web::WebRequest().get(lotmURL).listen([lotmInfo](web::WebResponse res) {
                if (!res.ok()) return;
                auto json = res.json();
                if (!json) return;
                auto value = json.unwrap();
                auto name = value.get<std::string>("name").unwrapOr("Unknown Level");
                auto creator = value.get<std::string>("creator").unwrapOr("Unknown Creator");
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

        auto mainMenu = this->getChildByID("main-menu");
        if (!mainMenu) {
            log::warn("DanzGDPS: main-menu was not found");
            return true;
        }

        // Native GD-style square tile. Drop resources/danzgdps-lotm.png later
        // to replace the fallback with the custom LOTM artwork.
        auto tile = CCSprite::create("danzgdps-lotm.png");
        if (!tile) {
            tile = CCSprite::create("GJ_square01.png");
        }

        if (!tile) {
            log::warn("DanzGDPS: failed to create LOTM tile sprite");
            return true;
        }

        tile->setScale(1.0f);

        auto label = CCLabelBMFont::create("LOTM", "bigFont.fnt");
        label->setScale(0.62f);
        label->setPosition({tile->getContentSize().width / 2.f, 14.f});
        tile->addChild(label);

        auto button = CCMenuItemSpriteExtra::create(
            tile,
            this,
            menu_selector(DanzMenuLayer::onDanzGDPS)
        );
        button->setID("danzgdps-lotm-button");

        mainMenu->addChild(button);
        mainMenu->updateLayout();

        return true;
    }

    void onDanzGDPS(CCObject*) {
        DanzGDPSPopup::create()->show();
    }
};
