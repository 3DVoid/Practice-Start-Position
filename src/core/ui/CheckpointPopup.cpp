#include <Geode/Geode.hpp>
#include "../checkpoints/CheckpointManager.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "CheckpointPopup.hpp"

using namespace geode::prelude;

#include <Geode/ui/Popup.hpp>
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

void CheckpointPopup::showSavedPage(CCObject*) {
    m_savedPage->setVisible(true);
    m_recentPage->setVisible(false);
}

void CheckpointPopup::showRecentPage(CCObject*) {
    m_savedPage->setVisible(false);
    m_recentPage->setVisible(true);
}

void CheckpointPopup::refreshSavedSelection() {
    for (size_t i = 0; i < m_savedRowBgs.size(); ++i) {
        bool selected = static_cast<int>(i) == m_selectedSaved;
        m_savedRowBgs[i]->setColor(selected ? ccc3(90, 200, 90) : ccc3(255, 255, 255));
    }
}

void CheckpointPopup::onSelectSavedRow(CCObject* sender) {
    auto btn = static_cast<CCMenuItem*>(sender);
    m_selectedSaved = btn->getTag();
    refreshSavedSelection();
}

void CheckpointPopup::refreshRecentSelection() {
    for (size_t i = 0; i < m_recentRowBgs.size(); ++i) {
        bool selected = static_cast<int>(i) == m_selectedRecent;
        m_recentRowBgs[i]->setColor(selected ? ccc3(90, 200, 90) : ccc3(255, 255, 255));
    }
}

void CheckpointPopup::onSelectRecentRow(CCObject* sender) {
    auto btn = static_cast<CCMenuItem*>(sender);
    m_selectedRecent = btn->getTag();
    refreshRecentSelection();
}

void CheckpointPopup::onLoadBtn(CCObject*) {
    int actlevelID = 0;
    if (auto pl = PlayLayer::get(); pl && pl->m_level) {
        actlevelID = pl->m_level->m_levelID.value();
    }
    if (m_selectedSaved >= 0) {
        auto idx = static_cast<std::size_t>(m_selectedSaved);
        CPMGR::loadCP(actlevelID, idx);
    }
}

void CheckpointPopup::onSaveBtn(CCObject*) {
    int actlevelID = 0;
    if (auto pl = PlayLayer::get(); pl && pl->m_level) {
        actlevelID = pl->m_level->m_levelID.value();
    }
    if (m_selectedSaved >= 0) {
        auto idx = static_cast<std::size_t>(m_selectedSaved);
        CPMGR::saveCP(actlevelID, idx);
    }
}

void CheckpointPopup::onDeleteBtn(CCObject*) {
    int actlevelID = 0;
    if (auto pl = PlayLayer::get(); pl && pl->m_level) {
        actlevelID = pl->m_level->m_levelID.value();
    }
    if (m_selectedSaved >= 0) {
        auto idx = static_cast<std::size_t>(m_selectedSaved);
        CPMGR::removeCP(actlevelID, idx);
    }
}

CCNode* CheckpointPopup::buildSavedPage() {
    m_savedRowBgs.clear();
    int actlevelID = 0;
    if (auto pl = PlayLayer::get(); pl && pl->m_level) {
        actlevelID = pl->m_level->m_levelID.value();
    }
    auto panel = CCScale9Sprite::create("GJ_square05.png");
    panel->setContentSize({270.f, 260.f});
    panel->setOpacity(255);
    panel->setZOrder(0);
    std::vector<CCNode*> rowbgs;
    std::vector<CCNode*> hitboxes;
    int i = 0;
    auto rowMenu = CCMenu::create();
    rowMenu->setContentSize({ 230.f, 267.f });
    rowMenu->setAnchorPoint({ 0.f, 0.f });
    rowMenu->ignoreAnchorPointForPosition(true);
    for (auto const& cp : CPMGR::getSavedSlots(actlevelID)) {
        auto rowbg = CCScale9Sprite::create("GJ_square02.png");
        if (!rowbg) {
            geode::log::warn("Row Background Not Found! Skipping...");
            continue;
        }
        rowbg->setContentSize({228.f, 65.f});
        rowbg->setColor({ 255, 255, 255 });

        auto emptybg = CCScale9Sprite::create("GJ_square02.png");
        if (!emptybg) {
            geode::log::warn("Empty Background Not Found! Skipping...");
            continue;
        }
        emptybg->setContentSize({228.f, 65.f});
        emptybg->setOpacity(0);

        auto hitbox = CCMenuItemSpriteExtra::create(emptybg, this, menu_selector(CheckpointPopup::onSelectSavedRow));
        hitbox->setTag((i));
        ++i;
        m_savedRowBgs.push_back(rowbg);

        rowMenu->addChild(hitbox);
        hitboxes.push_back(hitbox);

        CCNode*icon = CCSprite::createWithSpriteFrameName("checkpoint_01_color_001.png");
        if (!icon) {
            geode::log::warn("Checkpoint Icon Not Found! Replacing...");
            icon = CCLabelBMFont::create(fmt::format("!").c_str(), "goldFont.fnt");
        }
        auto idim = icon->getContentSize();
        float is = 22.f / idim.height;
        icon->setScale(is);

        auto perc = CCLabelBMFont::create(fmt::format("{}%", cp.perc).c_str(), "bigFont.fnt");
        auto pdim = perc->getContentSize();
        float ps = 16.f / pdim.height;
        perc->setScale(ps);

        CCNode* speed = CCSprite::createWithSpriteFrameName(Icon(getSpeed(cp.p1.speedm)));
        if (!speed) {
            geode::log::warn("Speed Icon Not Found!");
            speed = CCLabelBMFont::create(fmt::format("!").c_str(), "goldFont.fnt");
        }
        auto sdim = speed->getContentSize();
        float ss = 16.f / sdim.height;
        speed->setScale(ss);

        // create button, create pages for load and save

        auto content = CCNode::create();
        content->setContentSize({ 220.f, 65.f });
        content->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Start)->setCrossAxisAlignment(AxisAlignment::Center)->setGap(6.f));
        content->addChild(icon);
        content->addChild(perc);
        content->addChild(speed);
        content->updateLayout();

        rowbg->addChildAtPosition(content, Anchor::Center);
        rowbgs.push_back(rowbg);
    }

    auto list = ColumnLayout::create();
    list->setAxisAlignment(AxisAlignment::Start);
    list->setGap(4.f);

    auto contents = CCNode::create();
    contents->setContentSize({ 230.f, 267.f });
    contents->setAnchorPoint({ 0.5f, 0.5f });
    contents->setLayout(list);

    for (auto* rowbg : rowbgs) {
        contents->addChild(rowbg);
    }
    contents->updateLayout();

    contents->addChild(rowMenu);
    rowMenu->setPosition({ 0.f, 0.f });

    for (size_t i = 0; i < rowbgs.size() && i < hitboxes.size(); ++i) {
        hitboxes[i]->setPosition(rowbgs[i]->getPosition());
    }

    auto scroll = ScrollLayer::create({ 230.f, 267.f });
    scroll->m_contentLayer->addChild(contents);
    scroll->moveToTop();

    panel->addChildAtPosition(scroll, Anchor::Center);
    return panel;
}

CCNode* CheckpointPopup::buildRecentPage() {
    m_recentRowBgs.clear();
    int actlevelID = 0;
    if (auto pl = PlayLayer::get(); pl && pl->m_level) {
        actlevelID = pl->m_level->m_levelID.value();
    }
    auto panel = CCScale9Sprite::create("GJ_square05.png");
    panel->setContentSize({270.f, 260.f});
    panel->setOpacity(255);
    panel->setZOrder(0);
    std::vector<CCNode*> rowbgs;
    std::vector<CCNode*> hitboxes;
    int i = 0;
    auto rowMenu = CCMenu::create();
    rowMenu->setContentSize({ 230.f, 267.f });
    rowMenu->setAnchorPoint({ 0.f, 0.f });
    rowMenu->ignoreAnchorPointForPosition(true);
    for (auto const& cp : CPMGR::getList()) {
        auto rowbg = CCScale9Sprite::create("GJ_square02.png");
        if (!rowbg) {
            geode::log::warn("Row Background Not Found! Skipping...");
            continue;
        }
        rowbg->setContentSize({228.f, 65.f});
        rowbg->setColor({ 255, 255, 255 });

        auto emptybg = CCScale9Sprite::create("GJ_square02.png");
        if (!emptybg) {
            geode::log::warn("Empty Background Not Found! Skipping...");
            continue;
        }
        emptybg->setContentSize({228.f, 65.f});
        emptybg->setOpacity(0);

        auto hitbox = CCMenuItemSpriteExtra::create(emptybg, this, menu_selector(CheckpointPopup::onSelectRecentRow));
        hitbox->setTag((i));
        ++i;
        m_recentRowBgs.push_back(rowbg);

        rowMenu->addChild(hitbox);
        hitboxes.push_back(hitbox);

        CCNode*icon = CCSprite::createWithSpriteFrameName("checkpoint_01_color_001.png");
        if (!icon) {
            geode::log::warn("Checkpoint Icon Not Found!");
            icon = CCLabelBMFont::create(fmt::format("!").c_str(), "goldFont.fnt");
        }
        auto idim = icon->getContentSize();
        float is = 16.f / idim.height;
        icon->setScale(is);

        auto perc = CCLabelBMFont::create(fmt::format("{}%", cp.perc).c_str(), "bigFont.fnt");
        auto pdim = perc->getContentSize();
        float ps = 16.f / pdim.height;
        perc->setScale(ps);

        CCNode* speed = CCSprite::createWithSpriteFrameName(Icon(getSpeed(cp.p1.speedm)));
        if (!speed) {
            geode::log::warn("Speed Icon Not Found!");
            speed = CCLabelBMFont::create(fmt::format("!").c_str(), "goldFont.fnt");
        }
        auto sdim = speed->getContentSize();
        float ss = 16.f / sdim.height;
        speed->setScale(ss);

        // create button, create pages for load and save

        auto content = CCNode::create();
        content->setContentSize({ 220.f, 65.f });
        content->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Start)->setCrossAxisAlignment(AxisAlignment::Center)->setGap(6.f));
        content->addChild(icon);
        content->addChild(perc);
        content->addChild(speed);
        content->updateLayout();

        rowbg->addChildAtPosition(content, Anchor::Center);
        rowbgs.push_back(rowbg);
    }

    auto list = ColumnLayout::create();
    list->setAxisAlignment(AxisAlignment::Start);
    list->setGap(4.f);

    auto contents = CCNode::create();
    contents->setContentSize({ 230.f, 267.f });
    contents->setLayout(list);

    for (auto* rowbg : rowbgs) {
        contents->addChild(rowbg);
    }
    contents->updateLayout();

    contents->addChild(rowMenu);
    rowMenu->setPosition({ 0.f, 0.f });

    for (size_t i = 0; i < rowbgs.size() && i < hitboxes.size(); ++i) {
        hitboxes[i]->setPosition(rowbgs[i]->getPosition());
    }

    auto scroll = ScrollLayer::create({ 230.f, 267.f });
    scroll->m_contentLayer->addChild(contents);
    scroll->moveToTop();

    panel->addChildAtPosition(scroll, Anchor::Center);
    return panel;
}

bool CheckpointPopup::init() {
    if (!Popup::init(300.f, 300.f)) return false;

    /* auto bg = CCScale9Sprite::create("GJ_square05.png");
    bg->setContentSize({240.f, 255.f});
    bg->setLayout(ColumnLayout::create()->setGap(5.f));
    panel->addChildAtPosition(bg, Anchor::Center); */
    auto savedBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Saved"), this, menu_selector(CheckpointPopup::showSavedPage));
    auto recentBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Recent"), this, menu_selector(CheckpointPopup::showRecentPage));
    auto tabstop = CCMenu::create(recentBtn, savedBtn, nullptr);
    tabstop->alignItemsHorizontallyWithPadding(12.f);
    m_mainLayer->addChildAtPosition(tabstop, Anchor::Top, { 0.f, -25.f });
    tabstop->setZOrder(2);

    auto loadBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Load"), this, menu_selector(CheckpointPopup::onLoadBtn));
    auto saveBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Save"), this, menu_selector(CheckpointPopup::onSaveBtn));
    auto deleteBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Delete"), this, menu_selector(CheckpointPopup::onDeleteBtn));
    auto tabsbot = CCMenu::create(loadBtn, saveBtn, deleteBtn, nullptr);
    tabsbot->alignItemsHorizontallyWithPadding(12.f);
    m_mainLayer->addChildAtPosition(tabsbot, Anchor::Bottom, { 0.f, 25.f });
    tabsbot->setZOrder(2);


    m_savedPage = CCNode::create();
    m_savedPage->setContentSize({230.f, 267.f});
    m_mainLayer->addChildAtPosition(m_savedPage, Anchor::Center);
    m_savedPage->setZOrder(1);

    m_recentPage = CCNode::create();
    m_recentPage->setContentSize({230.f, 267.f});
    m_mainLayer->addChildAtPosition(m_recentPage, Anchor::Center);
    m_recentPage->setZOrder(1);


    m_recentPage->addChild(buildRecentPage());
    m_savedPage->addChild(buildSavedPage());

    m_recentPage->setVisible(true);
    m_savedPage->setVisible(false);

    return true;
}
CheckpointPopup* CheckpointPopup::create() {
    auto ret = new CheckpointPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}