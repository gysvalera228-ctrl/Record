#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(RecordPlayLayer, PlayLayer) {
    struct Fields {
        bool recording = false;
    };

    void onEnter() {
        PlayLayer::onEnter();

        if (this->getChildByID("record-control-menu"_spr))
            return;

        auto size = CCDirector::get()->getWinSize();
        auto menu = CCMenu::create();
        auto label = CCLabelBMFont::create("REC", "bigFont.fnt");

        if (!menu || !label)
            return;

        label->setScale(0.45f);
        label->setColor({255, 70, 70});

        auto button = CCMenuItemSpriteExtra::create(
            label,
            this,
            menu_selector(RecordPlayLayer::onToggleRecording)
        );

        if (!button)
            return;

        button->setID("record-toggle"_spr);
        menu->addChild(button);
        menu->setID("record-control-menu"_spr);
        menu->setPosition({size.width - 48.f, size.height - 82.f});

        this->addChild(menu, 1000);
    }

    void onToggleRecording(CCObject* sender) {
        m_fields->recording = !m_fields->recording;

        auto button = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto label = static_cast<CCLabelBMFont*>(button->getNormalImage());

        label->setString(m_fields->recording ? "STOP" : "REC");
    }
};

$on_mod(Loaded) {
    log::info("Record initialized.");
}
