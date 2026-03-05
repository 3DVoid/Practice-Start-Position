#pragma once

#include <vector>
#include <optional>
#include <cstddef>
#include "CheckpointStructure.hpp"
#include <Geode/modify/PlayLayer.hpp>

/** CP = Checkpoint
*   LCP = Level Checkpoint
*/

extern bool s_fromLoad;
extern bool s_doCheckpoint;

class CPMGR {
    public:
        static void onCPlaced(CheckpointData const& Checkpoint);

        static std::vector<CheckpointData> const& getList();

        static std::optional<CheckpointData> getCP(std::size_t index);

        static bool saveCP(int levelID, std::size_t index, bool editor);
        static bool loadCP(int levelID, std::size_t index, bool editor);
        static bool applyCP(PlayLayer* pl);
        static void clearList();
        static std::vector<CheckpointData> getSavedSlots(int levelID, bool editor);
        static bool removeCP(int levelID, std::size_t index, bool editor);
        static bool removeNotSavedCP(std::size_t index);
        
};