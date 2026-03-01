#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include "CheckpointLayer.hpp"
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/ui/ListView.hpp>
#include "../checkpoints/CheckpointManager.hpp"
#include "../checkpoints/CheckpointStructure.hpp"
#include <Geode/ui/Notification.hpp>

namespace {
    enum class Speeds {
        Slow,
        Normal,
        Two,
        Three,
        Fast
    };

    static Speeds getSpeed(double s) {
        if (s < 0.8) return Speeds::Slow;
        if (s < 1.0) return Speeds::Normal;
        if (s < 1.2) return Speeds::Two;
        if (s < 1.45) return Speeds::Three;
        return Speeds::Fast;
    };

    static char const* Icon(Speeds s) {
        if (s == Speeds::Slow) return "boost_01_001.png";
        if (s == Speeds::Normal) return "boost_02_001.png";
        if (s == Speeds::Two) return "boost_03_001.png";
        if (s == Speeds::Three) return "boost_04_001.png";
        return "boost_05_001.png";
    }
}

bool CheckpointLayer::init() {
    if (!CCLayerColor::initWithColor({ 0, 0, 0, 128 })) return false;
    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);
    this->setKeyboardEnabled(true);

    auto win = CCDirector::sharedDirector()->getWinSize();

    m_bgPanel = CCScale9Sprite::create("GJ_square05.png");
    if (!m_bgPanel) return false;
    m_bgPanel->setContentSize({ 450.f, 322.f });
    m_bgPanel->setAnchorPoint({ 0.5f, 0.5f });
    m_bgPanel->setPosition({ win.width / 2.f, win.height / 2.f });
    this->addChild(m_bgPanel);
    m_bgPanel->setZOrder(0);

    m_inPanel = CCScale9Sprite::create("GJ_square01.png");
    if (!m_inPanel) return false;
    m_inPanel->setAnchorPoint({ 0.5f, 0.5f});
    m_inPanel->setContentSize({ 382.5f, 255.f });
    m_inPanel->setPosition({ 225.f, 162.5f }); // change to 150 if it doesnt look right
    m_inPanel->setZOrder(1);

    m_bgPanel->addChild(m_inPanel);

    auto exitbtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"), this, menu_selector(CheckpointLayer::onCloseBtn));
    exitbtn->setZOrder(20);
    
    auto exitMenu = CCMenu::create(exitbtn, nullptr);
    exitMenu->setPosition({ 0.f, 0.f });
    exitMenu->setZOrder(20);
    m_bgPanel->addChild(exitMenu);
    exitbtn->setPosition({ 40.f, 302.f });

    auto RecentTabSprite = ButtonSprite::create("Recent");
    auto SavedTabSprite = ButtonSprite::create("Saved");

    m_recentTab = CCMenuItemSpriteExtra::create(RecentTabSprite, this, menu_selector(CheckpointLayer::onRecentTab));
    m_savedTab = CCMenuItemSpriteExtra::create(SavedTabSprite, this, menu_selector(CheckpointLayer::onSavedTab));

    auto toptabs = CCMenu::create(m_recentTab, m_savedTab, nullptr);
    toptabs->setZOrder(3);

    toptabs->alignItemsHorizontallyWithPadding(10.f);

    toptabs->setPosition({ 225.f, 300.f });

    auto DeleteTabSprite = ButtonSprite::create("Delete");
    auto LoadTabSprite = ButtonSprite::create("Load");
    auto SaveTabSprite = ButtonSprite::create("Save");

    m_deleteTab = CCMenuItemSpriteExtra::create(DeleteTabSprite, this, menu_selector(CheckpointLayer::onDeleteTab));
    m_saveTab = CCMenuItemSpriteExtra::create(SaveTabSprite, this, menu_selector(CheckpointLayer::onSaveTab));
    m_loadTab = CCMenuItemSpriteExtra::create(LoadTabSprite, this, menu_selector(CheckpointLayer::onLoadTab));

    auto bottabs = CCMenu::create(m_deleteTab, m_saveTab, m_loadTab, nullptr);
    bottabs->setZOrder(3);

    bottabs->alignItemsHorizontallyWithPadding(10.f);
    bottabs->setPosition({ 225.f, 22.f });

    m_bgPanel->addChild(toptabs);
    m_bgPanel->addChild(bottabs);

    m_recentPage = CCNode::create();
    if (!m_recentPage) return false;

    m_savedPage = CCNode::create();
    if (!m_savedPage) return false;

    m_recentPage->setAnchorPoint({0.f, 0.f});
    m_savedPage->setAnchorPoint({0.f, 0.f});
    m_recentPage->setPosition({0.f, 0.f});
    m_savedPage->setPosition({0.f, 0.f});


    m_recentPage->setContentSize({ 382.5f, 255.f });
    m_savedPage->setContentSize({ 382.5f, 255.f });

    if (auto recent = this->buildRecentTab()) {
        recent->setID("recent-list"_spr);
        m_recentPage->addChild(recent);
    }
    if (auto saved = this->buildSavedTab()) {
        saved->setID("saved-list"_spr);
        m_savedPage->addChild(saved);
    }

    m_inPanel->addChild(m_recentPage);
    m_inPanel->addChild(m_savedPage);

    onRecentTab(nullptr);

    return true;
}

void CheckpointLayer::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -99999, true);
}

bool CheckpointLayer::ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    return true;
}

CheckpointLayer* CheckpointLayer::create() {
    auto ret = new CheckpointLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCNode* CheckpointLayer::buildRecentTab() {
    m_recentRowButtons.clear();
    if (!m_inPanel) return nullptr;
    auto items = cocos2d::CCArray::create();
    auto list = CPMGR::getList();
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& cp = list[i];

        auto rowSpr = CCScale9Sprite::create("GJ_square04.png");
        if (!rowSpr) {
            geode::log::error("Failed to create row sprite");
            continue;
        }
        rowSpr->setContentSize({ 375.f, 48.f });
        
        auto rowBtn = CCMenuItemSpriteExtra::create(rowSpr, this, menu_selector(CheckpointLayer::onRowRecentClick));
        rowBtn->setAnchorPoint({ 0.5f, 0.5f });
        rowBtn->setTag(i);

        auto indexLabel = CCLabelBMFont::create(fmt::format("{}", i + 1).c_str(), "goldFont.fnt");
        
        auto checIcon = CCSprite::createWithSpriteFrameName("checkpoint_01_001.png");

        auto percLabel = CCLabelBMFont::create(fmt::format("{}%", cp.perc).c_str(), "goldFont.fnt");

        auto speedIcon = CCSprite::createWithSpriteFrameName(Icon(getSpeed(cp.p1.speed)));

        auto content = CCNode::create();
        content->addChild(checIcon);
        content->addChild(percLabel);
        content->addChild(speedIcon);
        content->setContentSize({ 375.f, 48.f });

        rowBtn->addChild(content);

        auto rowLayout = RowLayout::create();
        rowLayout->setAxisAlignment(AxisAlignment::Start);
        rowLayout->setGap(15.f);
        content->setID("row-content"_spr);
        content->setZOrder(6); // idk bro

        content->setLayout(rowLayout);
        content->updateLayout();
        content->setPositionX(9.f);
        indexLabel->setAnchorPoint({ 1.f, 0.5f });
        indexLabel->setPosition({ rowBtn->getContentSize().width - 8.f, 24.f });
        indexLabel->setID("index-label"_spr);
        indexLabel->setZOrder(6);
        rowBtn->addChild(indexLabel);

        auto rowMenu = CCMenu::create(rowBtn, nullptr);
        rowMenu->setPosition({ 191.25f, 26.f });

        auto item = CCNode::create();
        item->setContentSize({ 382.5f, 52.f });
        
        item->addChild(rowMenu);
        items->addObject(item);

        m_recentRowButtons.push_back(rowBtn);
    }
    auto listView = ListView::create(items, 52.f, 382.5f, 253.f);
    if (!listView) return nullptr;

    listView->setAnchorPoint({ 0.f, 0.f });
    listView->setPosition({ 0.f, 0.f });
    listView->setCellOpacity(0);

    return listView;
}

CCNode* CheckpointLayer::buildSavedTab() {
    m_savedRowButtons.clear();
    if (!m_inPanel) return nullptr;
    auto items = cocos2d::CCArray::create();
    auto playlayer = PlayLayer::get();
    if (!playlayer || !playlayer->m_level) return nullptr;
    auto list = CPMGR::getSavedSlots(playlayer->m_level->m_levelID);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& cp = list[i];

        auto rowSpr = CCScale9Sprite::create("GJ_square04.png");
        if (!rowSpr) {
            geode::log::error("Failed to create row sprite");
            continue;
        }
        rowSpr->setContentSize({ 375.f, 48.f });
        
        auto rowBtn = CCMenuItemSpriteExtra::create(rowSpr, this, menu_selector(CheckpointLayer::onRowSavedClick));
        rowBtn->setAnchorPoint({ 0.5f, 0.5f });
        rowBtn->setTag(i);

        auto indexLabel = CCLabelBMFont::create(fmt::format("{}", i + 1).c_str(), "goldFont.fnt");
        
        auto checIcon = CCSprite::createWithSpriteFrameName("checkpoint_01_001.png");

        auto percLabel = CCLabelBMFont::create(fmt::format("{}%", cp.perc).c_str(), "goldFont.fnt");

        auto speedIcon = CCSprite::createWithSpriteFrameName(Icon(getSpeed(cp.p1.speed)));

        auto content = CCNode::create();
        content->addChild(checIcon);
        content->addChild(percLabel);
        content->addChild(speedIcon);
        content->setContentSize({ 375.f, 48.f });

        rowBtn->addChild(content);

        auto rowLayout = RowLayout::create();
        rowLayout->setAxisAlignment(AxisAlignment::Start);
        rowLayout->setGap(15.f);
        content->setID("row-content"_spr);
        content->setZOrder(6); // idk bro

        content->setLayout(rowLayout);
        content->updateLayout();
        content->setPositionX(9.f);
        indexLabel->setAnchorPoint({ 1.f, 0.5f });
        indexLabel->setPosition({ rowBtn->getContentSize().width - 8.f, 24.f });
        indexLabel->setID("index-label"_spr);
        indexLabel->setZOrder(6);
        rowBtn->addChild(indexLabel);

        auto rowMenu = CCMenu::create(rowBtn, nullptr);
        rowMenu->setPosition({ 191.25f, 26.f });

        auto item = CCNode::create();
        item->setContentSize({ 382.5f, 52.f });
        
        item->addChild(rowMenu);
        items->addObject(item);

        m_savedRowButtons.push_back(rowBtn);
    }
    auto listView = ListView::create(items, 52.f, 382.5f, 253.f);
    if (!listView) return nullptr;

    listView->setAnchorPoint({ 0.f, 0.f });
    listView->setPosition({ 0.f, 0.f });
    listView->setCellOpacity(0);

    return listView;
}

void CheckpointLayer::onRowRecentClick(cocos2d::CCObject* sender) {
    auto clickedbtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
    if (!clickedbtn) return;

    m_selectedRecentRow = clickedbtn->getTag();

    for (auto* btn : m_recentRowButtons) {
        if (!btn) continue;

        auto bg = CCScale9Sprite::create(btn != clickedbtn ? "GJ_square04.png" : "GJ_square03.png");
        bg->setContentSize({ 375.f, 48.f });
        btn->setNormalImage(bg);

        if (auto content = btn->getChildByID("row-content"_spr)) {
            btn->reorderChild(content, 6);
        }
        if (auto indexLabel = btn->getChildByID("index-label"_spr)) {
            btn->reorderChild(indexLabel, 6);
        }
    }
}

void CheckpointLayer::onRowSavedClick(cocos2d::CCObject* sender) {
    auto clickedbtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
    if (!clickedbtn) return;

    m_selectedSavedRow = clickedbtn->getTag();

    for (auto* btn : m_savedRowButtons) {
        if (!btn) continue;

        auto bg = CCScale9Sprite::create(btn != clickedbtn ? "GJ_square04.png" : "GJ_square03.png");
        bg->setContentSize({ 375.f, 48.f });
        btn->setNormalImage(bg);

        if (auto content = btn->getChildByID("row-content"_spr)) {
            btn->reorderChild(content, 6);
        }
        if (auto indexLabel = btn->getChildByID("index-label"_spr)) {
            btn->reorderChild(indexLabel, 6);
        }
    }
}

void CheckpointLayer::onCloseBtn(cocos2d::CCObject*) {
    this->removeFromParentAndCleanup(true);
}

void CheckpointLayer::onRecentTab(cocos2d::CCObject*) {
    geode::log::info("RecentTab={}, SavedTab={}", (void*)m_recentTab, (void*)m_savedTab);
    if (!m_recentPage || !m_savedPage || !m_recentTab || !m_savedTab || !m_deleteTab || !m_saveTab || !m_loadTab) return;
    m_selectedPage = m_recentPage;
    m_recentPage->setVisible(true);
    m_savedPage->setVisible(false);
    auto recentImg = m_recentTab->getNormalImage();
    auto savedImg = m_savedTab->getNormalImage();
    auto saveImg = m_saveTab->getNormalImage();
    auto loadImg = m_loadTab->getNormalImage();

    if (auto spr = typeinfo_cast<ButtonSprite*>(savedImg)) spr->setColor({ 160, 160, 160 });
    if (auto spr = typeinfo_cast<ButtonSprite*>(recentImg)) spr->setColor({ 255, 255, 255 });
    if (auto spr = typeinfo_cast<ButtonSprite*>(loadImg)) spr->setColor({ 130, 130, 130 });
    if (auto spr = typeinfo_cast<ButtonSprite*>(saveImg)) spr->setColor({ 255, 255, 255 });

    m_loadTab->setEnabled(false);
    m_saveTab->setEnabled(true);
}

void CheckpointLayer::onSavedTab(cocos2d::CCObject*) {
    geode::log::info("RecentTab={}, SavedTab={}", (void*)m_recentTab, (void*)m_savedTab);
    if (!m_recentPage || !m_savedPage || !m_recentTab || !m_savedTab || !m_deleteTab || !m_saveTab || !m_loadTab) return;
    m_selectedPage = m_savedPage;
    m_recentPage->setVisible(false);
    m_savedPage->setVisible(true);

    auto recentImg = m_recentTab->getNormalImage();
    auto savedImg = m_savedTab->getNormalImage();
    auto saveImg = m_saveTab->getNormalImage();
    auto loadImg = m_loadTab->getNormalImage();

    if (auto spr = typeinfo_cast<ButtonSprite*>(recentImg)) spr->setColor({ 160, 160, 160 });
    if (auto spr = typeinfo_cast<ButtonSprite*>(savedImg)) spr->setColor({ 255, 255, 255 });
    if (auto spr = typeinfo_cast<ButtonSprite*>(saveImg)) spr->setColor({ 130, 130, 130 });
    if (auto spr = typeinfo_cast<ButtonSprite*>(loadImg)) spr->setColor({ 255, 255, 255 });

    m_saveTab->setEnabled(false);
    m_loadTab->setEnabled(true);
}

void CheckpointLayer::onSaveTab(cocos2d::CCObject*) {
    if (m_selectedPage == m_recentPage) {
        if (m_selectedRecentRow >= 0) {
            auto playlayer = PlayLayer::get();
            if (!playlayer || !playlayer->m_level) return;
            if (CPMGR::saveCP(playlayer->m_level->m_levelID, m_selectedRecentRow)) Notification::create("Success", NotificationIcon::Success)->show();
            else Notification::create("Failed To Save", NotificationIcon::Error)->show();
        }
        else Notification::create("Failed To Save", NotificationIcon::Error)->show();
    }
    else {
        Notification::create("Failed To Save", NotificationIcon::Error)->show();
        geode::log::error("Failed To Save, Selected Page = {}, Selected Row = {}", m_selectedPage, m_selectedRecentRow);
    }
}

void CheckpointLayer::onLoadTab(cocos2d::CCObject*) {
    if (m_selectedPage == m_savedPage) {
        if (m_selectedSavedRow >= 0 ) {
            auto playlayer = PlayLayer::get();
            if (!playlayer || !playlayer->m_level) return;
            if (CPMGR::loadCP(playlayer->m_level->m_levelID, m_selectedSavedRow)) Notification::create("Success", NotificationIcon::Success)->show();
            else {
                Notification::create("Failed To Load", NotificationIcon::Error)->show();
                geode::log::error("Failed To Load, Selected Page = {}, Selected Row = {}", m_selectedPage, m_selectedSavedRow);
            }
        }
        else {
            Notification::create("Failed To Load", NotificationIcon::Error)->show();
            geode::log::error("Failed To Load, Selected Page = {}, Selected Row = {}", m_selectedPage, m_selectedSavedRow);
        }
    }
    else {
        Notification::create("Failed To Load", NotificationIcon::Error)->show();
        geode::log::error("Failed To Load, Selected Page = {}, Selected Row = {}", m_selectedPage, m_selectedSavedRow);
    }
}

void CheckpointLayer::onDeleteTab(cocos2d::CCObject*) {
    if (m_selectedPage == m_savedPage) {
        if (m_selectedSavedRow >= 0 ) {
            auto playlayer = PlayLayer::get();
            if (!playlayer || !playlayer->m_level) return;
            if (!CPMGR::removeCP(playlayer->m_level->m_levelID, m_selectedSavedRow)) {
                Notification::create("Failed To Delete", NotificationIcon::Error)->show();
                geode::log::error("Failed To Delete, Selected Page = {}, Selected Row = {}", m_selectedPage, m_selectedSavedRow);
                return;
            }

            m_savedPage->removeChildByID("saved-list"_spr);
            if (auto saved = buildSavedTab()) {
                saved->setID("saved-list"_spr);
                m_savedPage->addChild(saved);
            }
            m_selectedSavedRow = -1;
        }
        else {
            Notification::create("Failed To Delete", NotificationIcon::Error)->show();
            geode::log::error("Failed To Delete, Selected Page = {}, Selected Row = {}", m_selectedPage, m_selectedSavedRow);
        }
    }
    else if (m_selectedPage == m_recentPage) {
        if (m_selectedRecentRow >= 0 ) {
            if (CPMGR::removeNotSavedCP(m_selectedRecentRow)) {
                m_recentPage->removeChildByID("recent-list"_spr);
                auto recent = buildRecentTab();
                if (!recent) {
                    Notification::create("Failed To Delete", NotificationIcon::Error)->show();
                    geode::log::error("Failed To Delete, Selected Page = {}, Selected Row = {}", m_selectedPage, m_selectedRecentRow);
                    return;
                }
                recent->setID("recent-list"_spr);
                m_recentPage->addChild(recent);
                m_selectedRecentRow = -1;
            }
            else {
                Notification::create("Failed To Delete", NotificationIcon::Error)->show();
                geode::log::error("Failed To Delete, Selected Page = {}, Selected Row = {}", m_selectedPage, m_selectedRecentRow);
            }
        }
        else {
            Notification::create("Failed To Delete", NotificationIcon::Error)->show();
            geode::log::error("Failed To Delete, Selected Page = {}, Selected Row = {}", m_selectedPage, m_selectedRecentRow);
        }
    }
    else {
        Notification::create("Failed To Delete", NotificationIcon::Error)->show();
        geode::log::error("Failed To Delete, Selected Page = {}, Selected Row = {}", m_selectedPage, m_selectedRecentRow);
    }
}

void CheckpointLayer::keyDown(cocos2d::enumKeyCodes key, double timestamp) {
    if (key == cocos2d::enumKeyCodes::KEY_Escape) {
        this->removeFromParentAndCleanup(true);
    }
    else {
        return;
    }
}