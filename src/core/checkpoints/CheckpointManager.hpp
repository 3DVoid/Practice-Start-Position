#pragma once

#include <vector>
#include <optional>
#include <cstddef>
#include "CheckpointStructure.hpp"

/** CP = Checkpoint
*   LCP = Level Checkpoint
*/

class CPMGR {
    public:
        static void onCPlaced(CheckpointData const& Checkpoint);

        static std::vector<CheckpointData> const& getList();

        static std::optional<CheckpointData> getCP(std::size_t index);

        static bool saveCP(int levelID, std::size_t index);
        static bool loadCP(int levelID, std::size_t index);
        static bool applyCP();
};