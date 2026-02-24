#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/PlayLayer.hpp>
#include "../checkpoints/CheckpointManager.hpp"

struct PL : Modify<PL, PlayLayer> {
    void onQuit() {
        PlayLayer::onQuit();
        CPMGR::clearList();
    }

    CheckpointObject* createCheckpoint() {
        auto out = PlayLayer::createCheckpoint();

        if (!m_player1 || !out) return out;

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
        cp.p1.speedm = m_player1->m_speedMultiplier;
        cp.p1.speed = m_player1->m_playerSpeed;
        cp.p1.gravity = m_player1->m_gravity;
        cp.p1.gravitymod = m_player1->m_gravityMod;
        
        cp.hasP2 = (out->m_player2Checkpoint != nullptr);
        if (m_player2 && cp.hasP2) {
            auto pos2 = m_player2->getPosition();
            cp.p2.x = pos2.x;
            cp.p2.y = pos2.y;
            cp.p2.vx = static_cast<float>(m_player2->getCurrentXVelocity());
            cp.p2.vy = static_cast<float>(m_player2->getYVelocity());
            cp.p2.rot = m_player2->getRotation();
            cp.p2.onGround = m_player2->m_isOnGround;
            cp.p2.upsideDown = m_player2->m_isUpsideDown;
            cp.p2.gamemode = m_player2->getActiveMode();
            cp.p2.speedm = m_player2->m_speedMultiplier;
            cp.p2.speed = m_player2->m_playerSpeed;
            cp.p2.gravity = m_player2->m_gravity;
            cp.p2.gravitymod = m_player2->m_gravityMod;
        }

        CPMGR::onCPlaced(cp);
        return out;
    }
};