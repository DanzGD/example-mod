#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

// Kita memodifikasi "EditorUI" (Tampilan antarmuka Editor)
class $modify(MyEditorUI, EditorUI) {
    
    // Fungsi init berjalan saat kita masuk ke Editor
    bool init(LevelEditorLayer* editorLayer) {
        // Jalankan kode asli game dulu agar editor muncul normal
        if (!EditorUI::init(editorLayer)) {
            return false;
        }

        // 1. Buat Menu (Wadah untuk tombol)
        auto myMenu = CCMenu::create();
        
        // Atur posisi menu (X=30 di kiri, Y=tinggi layar - 50 pixel)
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        myMenu->setPosition({30, winSize.height - 50});
        
        // Beri ID agar kompatibel dengan mod lain
        myMenu->setID("quick-save-menu"_spr);
        
        // Masukkan menu ke layar editor
        this->addChild(myMenu);

        // 2. Buat Gambar Tombol (Menggunakan gambar Save bawaan game)
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_saveBtn_001.png");
        sprite->setScale(0.6f); // Kecilkan sedikit agar tidak menghalangi

        // 3. Buat Tombolnya dan hubungkan ke fungsi "onQuickSave"
        auto btn = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(MyEditorUI::onQuickSave)
        );
        
        // Masukkan tombol ke dalam menu
        myMenu->addChild(btn);

        return true;
    }

    // Fungsi yang jalan saat tombol dipencet
    void onQuickSave(CCObject* sender) {
        // Memanggil fungsi asli "onSave" milik EditorUI
        this->onSave(nullptr);

        // Opsional: Munculkan notifikasi kecil (NotificationToast)
        // Agar kita tahu save berhasil tanpa popup yang mengganggu
        Notification::create(
            "Level Saved!", 
            NotificationIcon::Success, 
            0.5f // Durasi notifikasi
        )->show();
    }
};