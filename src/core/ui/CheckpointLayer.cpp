#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include "CheckpointLayer.hpp"
#include <Geode/modify/PlayLayer.hpp>
#include "../checkpoints/CheckpointManager.hpp"
#include "../checkpoints/CheckpointStructure.hpp"
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

    auto win = CCDirector::sharedDirector()->getWinSize();

    m_bgPanel = CCScale9Sprite::create("GJ_square05.png");
    if (!m_bgPanel) return false;
    m_bgPanel->setContentSize({ 450.f, 300.f });
    m_bgPanel->setPosition({ win.width / 2.f, win.height / 2.f });
    this->addChild(m_bgPanel);
    m_bgPanel->setZOrder(0);

    m_inPanel = CCScale9Sprite::create("GJ_square01.png");
    if (!m_inPanel) return false;
    m_inPanel->setAnchorPoint({ 0.5f, 0.5f});
    m_inPanel->setContentSize({ 382.5f, 255.f });
    m_inPanel->setPosition({ 0.f, 0.f });
    m_inPanel->setZOrder(1);

    m_bgPanel->addChild(m_inPanel);

    auto exitbtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"), this, menu_selector(CheckpointLayer::onCloseBtn));
    exitbtn->setZOrder(10);
    
    auto exitMenu = CCMenu::create(exitbtn, nullptr);
    exitMenu->setPosition({ 0.f, 0.f });
    m_bgPanel->addChild(exitMenu);
    exitbtn->setPosition({ -225.f, 150.f });

    auto RecentTabSprite = ButtonSprite::create("Recent Checkpoints");
    auto SavedTabSprite = ButtonSprite::create("Saved Checkpoints");

    auto RecentTab = CCMenuItemSpriteExtra::create(RecentTabSprite, this, menu_selector(CheckpointLayer::onRecentTab));
    auto SavedTab = CCMenuItemSpriteExtra::create(SavedTabSprite, this, menu_selector(CheckpointLayer::onSavedTab));

    auto toptabs = CCMenu::create(RecentTab, SavedTab, nullptr);
    toptabs->setZOrder(3);

    toptabs->alignItemsHorizontallyWithPadding(10.f);

    toptabs->setPositionY(278.f);
    m_bgPanel->addChild(toptabs);

    m_recentPage = CCNode::create();
    if (!m_recentPage) return false;

    m_savedPage = CCNode::create();
    if (!m_savedPage) return false;

    m_recentPage->setAnchorPoint({0.5f, 0.5f});
    m_savedPage->setAnchorPoint({0.5f, 0.5f});
    m_recentPage->setPosition({-191.25f, -127.5f});
    m_savedPage->setPosition({-191.25f, -127.5f});


    m_recentPage->setContentSize({ 382.5f, 255.f });
    m_savedPage->setContentSize({ 382.5f, 255.f });

    if (auto recent = this->buildRecentTab()) m_recentPage->addChild(recent);
    if (auto saved = this->buildSavedTab()) m_savedPage->addChild(saved);

    m_inPanel->addChild(m_recentPage);
    m_inPanel->addChild(m_savedPage);

    m_recentPage->setVisible(true);
    m_savedPage->setVisible(false);

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
    if (!m_inPanel) return nullptr;
    auto rowMenu = CCMenu::create();
    auto rows = CCNode::create();
    rows->setContentSize({ 382.5f, 680.f });
    rows->setAnchorPoint({ 0.f, 0.f });
    rows->setPosition({ 0.f, 0.f });
    for (auto const& cp : CPMGR::getList()) {
        auto rowSpr = CCScale9Sprite::create("GJ_square04.png");
        
        auto rowBtn = CCMenuItemSpriteExtra::create(rowSpr, this, menu_selector(CheckpointLayer::onRowClick));
        rowBtn->setContentSize({ 375.f, 48.f });
        rowBtn->setAnchorPoint({ 0.f, 0.f });

        
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
        rowLayout->setGap(10.f);

        content->setLayout(rowLayout);
        content->updateLayout();

        rowMenu->addChild(rowBtn);
    }

    auto rowMenuLayout = ColumnLayout::create();
    rowMenuLayout->setAxisAlignment(AxisAlignment::Start);
    rowMenuLayout->setGap(3.f);
    rowMenu->setContentSize(rows->getContentSize());
    rowMenu->setAnchorPoint({ 0.f, 0.f });
    rowMenu->setPosition({ 0.f, 0.f });
    rowMenu->setLayout(rowMenuLayout);
    rowMenu->updateLayout();
    rows->addChild(rowMenu);
    

    auto scrollLayer = ScrollLayer::create({ 382.5f, 255.f });

    scrollLayer->m_contentLayer->setContentSize(rows->getContentSize());
    scrollLayer->m_contentLayer->addChild(rows);

    scrollLayer->moveToTop();
    rowMenu->setZOrder(3);

    scrollLayer->setAnchorPoint({0.f, 0.f});
    scrollLayer->setPosition({0.f, 0.f});

    return scrollLayer;
}

CCNode* CheckpointLayer::buildSavedTab() {
    return nullptr;
}

void CheckpointLayer::onRowClick(cocos2d::CCObject* sender) {
    return;
}

void CheckpointLayer::onCloseBtn(cocos2d::CCObject*) {
    this->removeFromParentAndCleanup(true);
}

void CheckpointLayer::onRecentTab(cocos2d::CCObject*) {
    m_recentPage->setVisible(true);
    m_savedPage->setVisible(false);
}

void CheckpointLayer::onSavedTab(cocos2d::CCObject*) {
    m_recentPage->setVisible(false);
    m_savedPage->setVisible(true);
}