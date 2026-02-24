#pragma once

#include <vector>

#include <Geode/ui/Popup.hpp>


class CheckpointPopup : public geode::Popup {
    public:
        CCNode* m_savedPage = nullptr;
        CCNode* m_recentPage = nullptr;

        std::vector<geode::prelude::CCScale9Sprite*> m_savedRowBgs;
        int m_selectedSaved = -1;

        std::vector<geode::prelude::CCScale9Sprite*> m_recentRowBgs;
        int m_selectedRecent = -1;

        void showSavedPage(CCObject*);
        void showRecentPage(CCObject*);

        void refreshSavedSelection();
        void refreshRecentSelection();
        void onSelectSavedRow(CCObject*);
        void onSelectRecentRow(CCObject*);

        void onLoadBtn(CCObject*);
        void onSaveBtn(CCObject*);
        void onDeleteBtn(CCObject*);

        CCNode* buildSavedPage();
        CCNode* buildRecentPage();

        bool init();
        static CheckpointPopup* create();
};