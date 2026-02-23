#include "CheckpointManager.hpp"
#include <Geode/Geode.hpp>

namespace {
    std::vector<CheckpointData> s_list;
    std::optional<CheckpointData> s_loaded;
}

void CPMGR::onCPlaced(CheckpointData const& Checkpoint) {
    s_list.insert(s_list.begin(), Checkpoint);

    if (s_list.size() > 10) {
        s_list.pop_back();
    }

    geode::log::info("Checkpoint Data");

    for (std::size_t i = 0; i < s_list.size(); ++i) {
        auto const& cp = s_list[i];
        geode::log::info(
        "Index #: {}\n Player 1:\n x={}\n y={}\n vx={}\n vy={}\n rot={}\n ground={}\n upsideDown={}",
        i, cp.p1.x, cp.p1.y, cp.p1.vx, cp.p1.vy, cp.p1.rot, cp.p1.onGround, cp.p1.upsideDown
        );

        if (cp.hasP2) {
            geode::log::info(
            "\nPlayer 2:\n x={}\n y={}\n vx={}\n vy={}\n rot={}\n ground={}\n upsideDown={}",
            cp.p2.x, cp.p2.y, cp.p2.vx, cp.p2.vy, cp.p2.rot, cp.p2.onGround, cp.p2.upsideDown
        );
        } else {
            geode::log::info("No Player 2 Found!");
        }
        geode::log::info("percent={}", cp.perc);
    };
    
}

std::vector<CheckpointData> const& CPMGR::getList() {
    return s_list;
}
// might have to delete the = or sum
std::optional<CheckpointData> CPMGR::getCP(std::size_t index) {
    if (index >= s_list.size()) {
        return std::nullopt;
    }
    return s_list[index];
}

bool CPMGR::saveCP(int levelID, std::size_t index) {
    auto Checkpoint = getCP(index);
    if (!Checkpoint.has_value()) {
        return false;
    }

    // add the writing system to file
    (void)levelID;
    return true;
}

bool CPMGR::loadCP(int levelID, std::size_t index) {
    // read checkpoint from json file (maybe) and have it load the checkpoint, in json have it from 0-...

    (void)levelID;
    (void)index;
    s_loaded = std::nullopt;
    return false;
}

bool CPMGR::applyCP() {
    if (!s_loaded.has_value()) {
        return false;
    }

    // change state of player to the checkpoint state + set level to practice and place an actual checkpoint
    return true;
}