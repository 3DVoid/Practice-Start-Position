#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CheckpointLayer : public cocos2d::CCLayerColor {
public:
    static CheckpointLayer* create();
    CCScale9Sprite* m_bgPanel = nullptr;
    CCScale9Sprite* m_inPanel = nullptr;
    CCNode* m_recentPage = nullptr;
    CCNode* m_savedPage = nullptr;
    bool init() override;
    void onCloseBtn(cocos2d::CCObject*);
    void onRecentTab(cocos2d::CCObject*);
    void onSavedTab(cocos2d::CCObject*);
    CCNode* buildRecentTab();
    CCNode* buildSavedTab();
    void onRowClick(cocos2d::CCObject* sender);
    virtual void registerWithTouchDispatcher() override;
    virtual bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
};