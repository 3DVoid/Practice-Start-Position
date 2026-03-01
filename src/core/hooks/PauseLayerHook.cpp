#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/PauseLayer.hpp>
#include "../ui/CheckpointLayer.hpp"

struct BTL : Modify<BTL, PauseLayer> {
    void customSetup() {
        PauseLayer::customSetup();

        auto spr = CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(BTL::onBtn));

        auto menu = this->getChildByID("left-button-menu");
        menu->addChild(btn);

        btn->setID("practice-startpos-button"_spr);

        menu->updateLayout();
    }

    void onBtn(cocos2d::CCObject*) {
        auto layer = CheckpointLayer::create();
        if (!layer) return;
        layer->setID("practice-startpos-layer"_spr);

        this->addChild(layer, 10000);

    }
};