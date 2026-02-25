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

        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;
        if (scene->getChildByID("practice-startpos-layer"_spr)) return;
        layer->setID("practice-startpos-layer"_spr);

        scene->addChild(layer, 10000);

    }

    void keyDown(enumKeyCodes key, double timestamp) {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (scene && scene->getChildByID("practice-startpos-layer"_spr)) {
            if (key == cocos2d::enumKeyCodes::KEY_Escape) {
                scene->removeChildByID("practice-startpos-layer"_spr);
                return;
            }
            else {
                return;
            }
        }
        PauseLayer::keyDown(key, timestamp);
    }
};