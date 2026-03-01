#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CheckpointLayer : public cocos2d::CCLayerColor {
public:
    static CheckpointLayer* create();
    CCScale9Sprite* m_bgPanel = nullptr;
    CCScale9Sprite* m_inPanel = nullptr;
    CCNode* m_recentPage = nullptr;
    CCNode* m_savedPage = nullptr;
    CCMenuItemSpriteExtra* m_saveTab = nullptr;
    CCMenuItemSpriteExtra* m_loadTab = nullptr;
    CCMenuItemSpriteExtra* m_deleteTab = nullptr;
    CCMenuItemSpriteExtra* m_recentTab = nullptr;
    CCMenuItemSpriteExtra* m_savedTab = nullptr;
    int m_selectedRecentRow = -1;
    int m_selectedSavedRow = -1;
    CCNode* m_selectedPage = nullptr;
    std::vector<CCMenuItemSpriteExtra*> m_recentRowButtons;
    std::vector<CCMenuItemSpriteExtra*> m_savedRowButtons;
    bool init() override;
    void onCloseBtn(cocos2d::CCObject*);
    void onRecentTab(cocos2d::CCObject*);
    void onSavedTab(cocos2d::CCObject*);
    void onDeleteTab(cocos2d::CCObject*);
    void onSaveTab(cocos2d::CCObject*);
    void onLoadTab(cocos2d::CCObject*);
    CCNode* buildRecentTab();
    CCNode* buildSavedTab();
    void onRowRecentClick(cocos2d::CCObject* sender);
    void onRowSavedClick(cocos2d::CCObject* sender);
    virtual void registerWithTouchDispatcher() override;
    virtual bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void keyDown(cocos2d::enumKeyCodes key, double timestamp) override;
};