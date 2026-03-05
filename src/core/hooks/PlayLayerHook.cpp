#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/PlayLayer.hpp>
#include "../checkpoints/CheckpointManager.hpp"
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>

struct PL : Modify<PL, PlayLayer> {
    void onQuit() {
        PlayLayer::onQuit();
        CPMGR::clearList();
    }

    CheckpointObject* createCheckpoint() {
        auto out = PlayLayer::createCheckpoint();

        if (!m_player1 || !out || s_fromLoad || !out->m_player1Checkpoint) return out;

        CheckpointData cp{};
        auto pos1 = out->m_player1Checkpoint->m_position;
        cp.p1.x = pos1.x;
        cp.p1.y = pos1.y;
        cp.p1.vx = out->m_player1Checkpoint->m_platformerXVelocity;
        cp.p1.vy = out->m_player1Checkpoint->m_yVelocity;
        cp.p1.rot = out->m_player1Checkpoint->m_rotation;
        cp.p1.onGround = out->m_player1Checkpoint->m_isOnGround;
        cp.p1.upsideDown = out->m_player1Checkpoint->m_isUpsideDown;
        cp.p1.gamemode = static_cast<int>(m_player1->getActiveMode());
        cp.perc = this->getCurrentPercentInt();
        cp.p1.speedm = out->m_player1Checkpoint->m_speedMultiplier;
        cp.p1.speed = out->m_player1Checkpoint->m_playerSpeed;
        cp.p1.gravity = out->m_player1Checkpoint->m_gravity;
        cp.p1.gravitymod = out->m_player1Checkpoint->m_gravityMod;
        cp.p1.mini = out->m_player1Checkpoint->m_isMini;
        
        cp.hasP2 = (out->m_player2Checkpoint != nullptr);
        if (m_player2 && cp.hasP2 && out->m_player2Checkpoint) {
            auto pos2 = out->m_player2Checkpoint->m_position;
            cp.p2.x = pos2.x;
            cp.p2.y = pos2.y;
            cp.p2.vx = out->m_player2Checkpoint->m_platformerXVelocity;
            cp.p2.vy = out->m_player2Checkpoint->m_yVelocity;
            cp.p2.rot = out->m_player2Checkpoint->m_rotation;
            cp.p2.onGround = out->m_player2Checkpoint->m_isOnGround;
            cp.p2.upsideDown = out->m_player2Checkpoint->m_isUpsideDown;
            cp.p2.gamemode = static_cast<int>(m_player2->getActiveMode());
            cp.perc = this->getCurrentPercentInt();
            cp.p2.speedm = out->m_player2Checkpoint->m_speedMultiplier;
            cp.p2.speed = out->m_player2Checkpoint->m_playerSpeed;
            cp.p2.gravity = out->m_player2Checkpoint->m_gravity;
            cp.p2.gravitymod = out->m_player2Checkpoint->m_gravityMod;
            cp.p2.mini = out->m_player2Checkpoint->m_isMini;
        }
        cp.levelTime = this->m_gameState.m_levelTime;
        cp.seed = this->m_randomSeed;

        if (!this->m_level) return out;

        cp.editor = (this->m_level->m_levelType == GJLevelType::Editor || this->m_level->m_levelType == GJLevelType::Saved);

        auto ch = FMODAudioEngine::sharedEngine()->getMusicChannelID(0);
        if (ch >= 0) {
            cp.musicTime = FMODAudioEngine::sharedEngine()->getMusicTimeMS(ch);
        }

        CPMGR::onCPlaced(cp);
        return out;
    }

    /* void resetLevel() {
        PlayLayer::resetLevel();
        geode::log::info("ResetLevel called, pendingApply={}", s_pendingApply);
    
        if (s_pendingApply == true) {
            this->createCheckpoint();
            s_pendingApply = false;
            Loader::get()->queueInMainThread([this]() {
                // CPMGR::applyCP(this);
                m_player1->setPosition({1601.7975f, 713.16943f});
                geode::log::info("(in PL hook) Pos, X={}, Y={}", this->m_player1->m_positionX, this->m_player1->m_positionY);
                geode::log::info("m_player1 ptr in lambda: {}", (void*)m_player1);
            });
            
        }
    } */

    void resume() {
        PlayLayer::resume();

        if (s_doCheckpoint) {
            s_doCheckpoint = false;
            Loader::get()->queueInMainThread([this] {
                geode::log::info("About to createCheckpoint, player pos: {}, {}", m_player1->getPositionX(), m_player1->getPositionY());
                s_fromLoad = true;
                auto checkpoint = this->createCheckpoint();
                if (!checkpoint) {
                    geode::log::error("Error creating checkpoint when applying in PlayLayerHook.cpp");
                    s_fromLoad = false;
                    return;
                }
                this->m_currentCheckpoint = checkpoint;
                geode::log::info("M_CHECKPOINTARRY BF = {}", m_checkpointArray->count());
                this->storeCheckpoint(checkpoint);
                geode::log::info("M_CHECKPOINTARRY AT = {}", m_checkpointArray->count());
                geode::log::info("createCheckpoint returned: {}", (void*)checkpoint);
                geode::log::info("m_currentCheckpoint after: {}", (void*)m_currentCheckpoint);
                s_fromLoad = false;
            });
        }
        geode::log::info("On Resume getPosition: {}, {}", this->m_player1->getPosition().x, this->m_player1->getPosition().y);
        geode::log::info("On Resume, Pos X={}, Y={}", this->m_player1->m_positionX, this->m_player1->m_positionY);
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        geode::log::info("Pos In ResetLevel/Reset Level Just Now, Pos={}", this->m_player1->getPosition());
    }
};