#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

class $modify(RecordPauseLayer, PauseLayer) {
    struct Fields {
        bool panelOpen = false;
        CCNode* panel = nullptr;
    };

    void customSetup() {
        PauseLayer::customSetup();

        auto size = CCDirector::get()->getWinSize();

        auto label = CCLabelBMFont::create("Record", "bigFont.fnt");
        if (!label) return;

        label->setScale(0.45f);

        auto button = CCMenuItemSpriteExtra::create(
            label,
            this,
            menu_selector(RecordPauseLayer::onOpenRecord)
        );

        if (!button) return;

        auto menu = CCMenu::create();
        menu->addChild(button);
        menu->setPosition({size.width - 65.f, size.height * 0.5f});
        menu->setID("record-pause-menu"_spr);

        this->addChild(menu, 1000);
    }

    void onOpenRecord(CCObject*) {
        if (m_fields->panelOpen) {
            closePanel();
            return;
        }

        auto size = CCDirector::get()->getWinSize();

        auto panel = CCLayerColor::create(
            {20, 22, 28, 245},
            270.f,
            155.f
        );

        if (!panel) return;

        panel->setPosition({
            (size.width - 270.f) / 2.f,
            (size.height - 155.f) / 2.f
        });

        panel->setID("record-panel"_spr);

        auto title = CCLabelBMFont::create("RECORD", "bigFont.fnt");
        title->setScale(0.55f);
        title->setPosition({135.f, 125.f});
        panel->addChild(title);

        auto status = CCLabelBMFont::create(
            "Capture engine is being connected",
            "bigFont.fnt"
        );

        status->setScale(0.28f);
        status->setPosition({135.f, 90.f});
        status->setColor({180, 190, 205});
        panel->addChild(status);

        auto startLabel = CCLabelBMFont::create("START", "bigFont.fnt");
        startLabel->setScale(0.5f);

        auto startButton = CCMenuItemSpriteExtra::create(
            startLabel,
            this,
            menu_selector(RecordPauseLayer::onStart)
        );

        auto closeLabel = CCLabelBMFont::create("CLOSE", "bigFont.fnt");
        closeLabel->setScale(0.4f);

        auto closeButton = CCMenuItemSpriteExtra::create(
            closeLabel,
            this,
            menu_selector(RecordPauseLayer::onClose)
        );

        auto buttons = CCMenu::create();
        buttons->addChild(startButton);
        buttons->addChild(closeButton);

        startButton->setPosition({-65.f, -15.f});
        closeButton->setPosition({65.f, -15.f});

        panel->addChild(buttons);
        this->addChild(panel, 2000);

        m_fields->panel = panel;
        m_fields->panelOpen = true;
    }

    void onStart(CCObject*) {
        FLAlertLayer::create(
            "Record",
            "The video capture pipeline is not connected yet. "
            "Starting now would not produce a valid gameplay recording.",
            "OK"
        )->show();
    }

    void onClose(CCObject*) {
        closePanel();
    }

    void closePanel() {
        if (m_fields->panel) {
            m_fields->panel->removeFromParentAndCleanup(true);
            m_fields->panel = nullptr;
        }

        m_fields->panelOpen = false;
    }
};

$on_mod(Loaded) {
    log::info("Record initialized.");
}
