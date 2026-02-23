#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/PlayLayer.hpp>
#include "../checkpoints/CheckpointManager.hpp"

struct PL : Modify<PL, PlayLayer> {
    CheckpointObject* createCheckpoint() {
        auto out = PlayLayer::createCheckpoint();

        if (!m_player1) return out;

        CheckpointData cp{};
        auto pos1 = m_player1->getPosition();
        cp.p1.x = pos1.x;
        cp.p1.y = pos1.y;
        cp.p1.vx = static_cast<float>(m_player1->getCurrentXVelocity());
        cp.p1.vy = static_cast<float>(m_player1->getYVelocity());
        cp.p1.rot = m_player1->getRotation();
        cp.p1.onGround = m_player1->m_isOnGround;
        cp.p1.upsideDown = m_player1->m_isUpsideDown;
        cp.p1.gamemode = m_player1->getActiveMode();
        cp.perc = this->getCurrentPercentInt();
        
        // make a placeholder number for something absurdly high and then dont do this if the thing is at that number, bc this is not reliable
        if (m_player2->isVisible()) {
            cp.hasP2 = true;
            auto pos2 = m_player2->getPosition();
            cp.p2.x = pos2.x;
            cp.p2.y = pos2.y;
            cp.p2.vx = static_cast<float>(m_player2->getCurrentXVelocity());
            cp.p2.vy = static_cast<float>(m_player2->getYVelocity());
            cp.p2.rot = m_player2->getRotation();
            cp.p2.onGround = m_player2->m_isOnGround;
            cp.p2.upsideDown = m_player2->m_isUpsideDown;
            cp.p2.gamemode = m_player2->getActiveMode();
        }

        CPMGR::onCPlaced(cp);
        return out;
    }
};