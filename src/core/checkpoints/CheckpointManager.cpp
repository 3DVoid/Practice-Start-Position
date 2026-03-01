#include "CheckpointManager.hpp"
#include <Geode/Geode.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <Geode/modify/PlayLayer.hpp>

namespace {
    std::vector<CheckpointData> s_list;
    std::optional<CheckpointData> s_loaded;

    std::filesystem::path getCheckpointPath(int levelID) {
        auto dir = geode::Mod::get()->getSaveDir() / "saved-checkpoints";
        std::filesystem::create_directories(dir);
        return dir / (std::to_string(levelID) + ".json");
    }

    matjson::Value playerStateToJson(PlayerState const& ps) {
        return matjson::makeObject({
            { "x", ps.x },
            { "y", ps.y },
            { "vx", ps.vx },
            { "vy", ps.vy },
            { "rot", ps.rot },
            { "speed", ps.speed },
            { "speedm", ps.speedm },
            { "upsideDown", ps.upsideDown },
            { "onGround", ps.onGround },
            { "gamemode", static_cast<int>(ps.gamemode) },
            { "gravity", ps.gravity },
            { "gravitymod", ps.gravitymod }
        });
    }

    matjson::Value checkpointToJson(std::size_t index, CheckpointData const& cp) {
        std::string sn = "slot" + std::to_string(index);
        return matjson::makeObject({
            { "name", sn },
            { "perc", cp.perc },
            { "hasP2", cp.hasP2 },
            { "p1", playerStateToJson(cp.p1) },
            { "p2", cp.hasP2 ? playerStateToJson(cp.p2) : matjson::Value()}
        });
    }

    bool addSlot(matjson::Value& json, CheckpointData const& cp) {
        if (!json.contains("slots") || !json["slots"].isArray()) {
            json["slots"] = matjson::Value::array();
        }

        auto arrr = json["slots"].asArray();
        if (!arrr) return false;

        auto& arr = arrr.unwrap();
        std::size_t index = arr.size();
        arr.insert(arr.begin(), checkpointToJson(index, cp));

        return true;
    }

    bool writeJson(std::filesystem::path const& file, matjson::Value const& json) {
        std::ofstream out(file, std::ios::trunc);
        if (!out.is_open()) return false;
        out << json.dump(2);
        return out.good();
    }

    matjson::Value readOrCreateJSON(std::filesystem::path const& file, int levelID) {
        matjson::Value json = matjson::makeObject({
            { "levelID", levelID },
            { "slots", matjson::Value::array() }
        });

        if (!std::filesystem::exists(file)) {
            return json;
        }

        std::ifstream in(file);
        if (!in.is_open()) return json;

        std::stringstream buffer;
        buffer << in.rdbuf();

        auto parsed = matjson::parse(buffer.str());
        if (!parsed) return json;

        auto loaded = parsed.unwrap();

        if (!loaded.contains("slots") || !loaded["slots"].isArray()) {
            loaded["slots"] = matjson::Value::array();
        }
        if (!loaded.contains("levelID")) {
            loaded["levelID"] = levelID;
        }

        return loaded;
    }
    
    bool removeSlot(int levelID, int index) {
        auto file = getCheckpointPath(levelID);
        auto json = readOrCreateJSON(file, levelID);

        if (!json.contains("slots") || !json["slots"].isArray()) {
            json["slots"] = matjson::Value::array();
        }

        auto arrr = json["slots"].asArray();
        if (!arrr) return false;
        
        auto& arr = arrr.unwrap();

        if (index >= arr.size()) {
            geode::log::error("ERROR: Index Num Too High! Please Report to 3dvoidyt (Discord or GD) if this problem consists. Index={}, Array Size={}", index, arr.size());
            return false;
        }

        if (!arr.empty()) {
            arr.erase(arr.begin() + index);
        }

        return writeJson(file, json);
    }

    static GameObjectType modeToPortal(int mode) {
        switch (mode) {
            case 0: return GameObjectType::CubePortal;
            case 1: return GameObjectType::ShipPortal;
            case 2: return GameObjectType::BallPortal;
            case 3: return GameObjectType::UfoPortal;
            case 4: return GameObjectType::WavePortal;
            case 5: return GameObjectType::RobotPortal;
            case 6: return GameObjectType::SpiderPortal;
            case 7: return GameObjectType::SwingPortal;
            default: return GameObjectType::CubePortal;
        }
    }
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
        "Index #: {}\n Player 1:\n x={}\n y={}\n vx={}\n vy={}\n rot={}\n ground={}\n upsideDown={}\n speed={}\n gravity={}\n gravity mod={}\n speedm={}",
        i, cp.p1.x, cp.p1.y, cp.p1.vx, cp.p1.vy, cp.p1.rot, cp.p1.onGround, cp.p1.upsideDown, cp.p1.speed, cp.p1.gravity, cp.p1.gravitymod, cp.p1.speedm
        );

        if (cp.hasP2) {
            geode::log::info(
            "\nPlayer 2:\n x={}\n y={}\n vx={}\n vy={}\n rot={}\n ground={}\n upsideDown={}\n speed={}\n gravity={}\n gravity mod={}\n speedm={}",
            cp.p2.x, cp.p2.y, cp.p2.vx, cp.p2.vy, cp.p2.rot, cp.p2.onGround, cp.p2.upsideDown, cp.p2.speed, cp.p2.gravity, cp.p2.gravitymod, cp.p2.speedm
        );
        } else {
            geode::log::info("No Player 2 Found!");
        }
        geode::log::info("percent={}", cp.perc);
    }
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
    auto cp = getCP(index);
    if (!cp.has_value()) {
        return false;
    }

    auto file = getCheckpointPath(levelID);
    auto json = readOrCreateJSON(file, levelID);
    if(!addSlot(json, cp.value())) return false;
    return writeJson(file, json);
}

bool CPMGR::loadCP(int levelID, std::size_t index) {
    // read checkpoint from json file (maybe) and have it load the checkpoint, in json have it from 0-...
    auto file = getCheckpointPath(levelID);

    if (!std::filesystem::exists(file)) {
            geode::log::error("File Not Found While Loading. Please Report To 3dvoidyt (Discord or GD) If Error Consists. File={}", file);
            return false;
    }

    std::ifstream in(file);
    if (!in.is_open()) {
        geode::log::error("Bad Permissions. Please Report To 3dvoidyt (Discord or GD) If Error Consists.");
        return false;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();

    auto parsed = matjson::parse(buffer.str());
    if (!parsed) {
        geode::log::error("Unable To Parse While Loading. Please Report To 3dvoidyt (Discord or GD) If Error Consists. File={}", file);
        return false;
    };

    auto jsontext = parsed.unwrap();

    auto arrr = jsontext["slots"].asArray();
    if (!arrr) return false;
    auto& arr = arrr.unwrap();
    if (index >= arr.size()) return false;
    auto const& slot = arr[index];

    CheckpointData loadedcp{};

    if (auto v = slot["perc"].asInt()) loadedcp.perc = v.unwrap();
    if (auto v = slot["hasP2"].asBool()) loadedcp.hasP2 = v.unwrap();

    auto const& p1 = slot["p1"];
    if (auto v = p1["x"].asDouble()) loadedcp.p1.x = static_cast<float>(v.unwrap());
    if (auto v = p1["y"].asDouble()) loadedcp.p1.y = static_cast<float>(v.unwrap());
    if (auto v = p1["vx"].asDouble()) loadedcp.p1.vx = static_cast<float>(v.unwrap());
    if (auto v = p1["vy"].asDouble()) loadedcp.p1.vy = static_cast<float>(v.unwrap());
    if (auto v = p1["rot"].asDouble()) loadedcp.p1.rot = static_cast<float>(v.unwrap());
    if (auto v = p1["upsideDown"].asBool()) loadedcp.p1.upsideDown = v.unwrap();
    if (auto v = p1["onGround"].asBool()) loadedcp.p1.onGround = v.unwrap();
    if (auto v = p1["speed"].asDouble()) loadedcp.p1.speed = v.unwrap();
    if (auto v = p1["speedm"].asDouble()) loadedcp.p1.speedm = v.unwrap();
    if (auto v = p1["gravity"].asDouble()) loadedcp.p1.gravity = v.unwrap();
    if (auto v = p1["gravitymod"].asDouble()) loadedcp.p1.gravitymod = static_cast<float>(v.unwrap());
    if (auto v = p1["gamemode"].asInt()) loadedcp.p1.gamemode = v.unwrap();

    if (loadedcp.hasP2 && slot["p2"].isObject()) {
        auto const& p2 = slot["p2"];
        if (auto v = p2["x"].asDouble()) loadedcp.p2.x = static_cast<float>(v.unwrap());
        if (auto v = p2["y"].asDouble()) loadedcp.p2.y = static_cast<float>(v.unwrap());
        if (auto v = p2["vx"].asDouble()) loadedcp.p2.vx = static_cast<float>(v.unwrap());
        if (auto v = p2["vy"].asDouble()) loadedcp.p2.vy = static_cast<float>(v.unwrap());
        if (auto v = p2["rot"].asDouble()) loadedcp.p2.rot = static_cast<float>(v.unwrap());
        if (auto v = p2["upsideDown"].asBool()) loadedcp.p2.upsideDown = v.unwrap();
        if (auto v = p2["onGround"].asBool()) loadedcp.p2.onGround = v.unwrap();
        if (auto v = p2["speed"].asDouble()) loadedcp.p2.speed = v.unwrap();
        if (auto v = p2["speedm"].asDouble()) loadedcp.p2.speedm = v.unwrap();
        if (auto v = p2["gravity"].asDouble()) loadedcp.p2.gravity = v.unwrap();
        if (auto v = p2["gravitymod"].asDouble()) loadedcp.p2.gravitymod = static_cast<float>(v.unwrap());
        if (auto v = p2["gamemode"].asInt()) loadedcp.p2.gamemode = v.unwrap();
    }

    s_loaded = loadedcp;

    auto pl = PlayLayer::get();
    if (!pl) {
        geode::log::error("Unable To Auto-Apply, No PlayLayer Found. Please Report To 3dvoidyt (Discord or GD) If Error Consists.");
        return false;
    }
    
    return applyCP(pl);
}

bool CPMGR::applyCP(PlayLayer* pl) {
    if (!pl) return false;
    if (!s_loaded) return false;
    if (!pl->m_player1) return false;

    auto const& cp = s_loaded.value();

    pl->toggleDualMode(nullptr, cp.hasP2, pl->m_player1, true);
    if (!pl->m_isPracticeMode) pl->togglePracticeMode(true);
    if (cp.hasP2 && pl->m_player2) pl->m_player2->switchedToMode(modeToPortal(cp.p2.gamemode));

    geode::log::info("Applying CP: x={}, y={}, perc={}", cp.p1.x, cp.p1.y, cp.perc);

    pl->m_player1->switchedToMode(modeToPortal(cp.p1.gamemode));
    pl->m_player1->setPosition({ cp.p1.x, cp.p1.y });
    pl->m_player1->setYVelocity(cp.p1.vy, 0);
    if (pl->m_isPlatformer) {
        pl->m_player1->m_platformerXVelocity = cp.p1.vx;
    }
    pl->m_player1->setRotation(cp.p1.rot);
    pl->m_player1->flipGravity(cp.p1.upsideDown, true);
    pl->m_player1->m_isOnGround = cp.p1.onGround;
    pl->m_player1->m_playerSpeed = cp.p1.speed;
    pl->m_player1->m_speedMultiplier = cp.p1.speedm;
    pl->m_player1->m_gravity = cp.p1.gravity;
    pl->m_player1->m_gravityMod = cp.p1.gravitymod;

    pl->m_player1->updateRotation(0.f);

    pl->m_player1->setPosition({ cp.p1.x, cp.p1.y });
    pl->m_player1->setYVelocity(cp.p1.vy, 0);

    if (cp.hasP2 && !pl->m_player2) {
        geode::log::error("CP Error, idk bro im tired");
    }
    if (cp.hasP2 && pl->m_player2 != nullptr) {
        pl->m_player2->setPosition({ cp.p2.x, cp.p2.y });
        pl->m_player2->setYVelocity(cp.p2.vy, 0);
        if (pl->m_isPlatformer) {
            pl->m_player2->m_platformerXVelocity = cp.p2.vx;
        }
        pl->m_player2->setRotation(cp.p2.rot);
        pl->m_player2->flipGravity(cp.p2.upsideDown, true);
        pl->m_player2->m_isOnGround = cp.p2.onGround;
        pl->m_player2->m_playerSpeed = cp.p2.speed;
        pl->m_player2->m_speedMultiplier = cp.p2.speedm;
        pl->m_player2->m_gravity = cp.p2.gravity;
        pl->m_player2->m_gravityMod = cp.p2.gravitymod;

        pl->m_player2->updateRotation(0.f);

        pl->m_player2->setPosition({ cp.p2.x, cp.p2.y });
        pl->m_player2->setYVelocity(cp.p2.vy, 0);
    }
    
    return true;
}

void CPMGR::clearList() {
    s_list.clear();
    s_loaded = std::nullopt;
}

std::vector<CheckpointData> CPMGR::getSavedSlots(int levelID) {
    std::vector<CheckpointData> out;

    auto file = getCheckpointPath(levelID);
    auto json = readOrCreateJSON(file, levelID);

    auto arrr = json["slots"].asArray();
    if (!arrr) return out;

    auto const& arr = arrr.unwrap();
    
    out.reserve(arr.size());

    for (auto const& slot : arr) {
        CheckpointData loadedcp{};

        if (auto v = slot["perc"].asInt()) loadedcp.perc = v.unwrap();
        if (auto v = slot["hasP2"].asBool()) loadedcp.hasP2 = v.unwrap();

        auto const& p1 = slot["p1"];
        if (auto v = p1["x"].asDouble()) loadedcp.p1.x = static_cast<float>(v.unwrap());
        if (auto v = p1["y"].asDouble()) loadedcp.p1.y = static_cast<float>(v.unwrap());
        if (auto v = p1["vx"].asDouble()) loadedcp.p1.vx = static_cast<float>(v.unwrap());
        if (auto v = p1["vy"].asDouble()) loadedcp.p1.vy = static_cast<float>(v.unwrap());
        if (auto v = p1["rot"].asDouble()) loadedcp.p1.rot = static_cast<float>(v.unwrap());
        if (auto v = p1["upsideDown"].asBool()) loadedcp.p1.upsideDown = v.unwrap();
        if (auto v = p1["onGround"].asBool()) loadedcp.p1.onGround = v.unwrap();
        if (auto v = p1["speed"].asDouble()) loadedcp.p1.speed = v.unwrap();
        if (auto v = p1["speedm"].asDouble()) loadedcp.p1.speedm = v.unwrap();
        if (auto v = p1["gravity"].asDouble()) loadedcp.p1.gravity = v.unwrap();
        if (auto v = p1["gravitymod"].asDouble()) loadedcp.p1.gravitymod = static_cast<float>(v.unwrap());
        if (auto v = p1["gamemode"].asInt()) loadedcp.p1.gamemode = v.unwrap();

        if (loadedcp.hasP2 && slot["p2"].isObject()) {
            auto const& p2 = slot["p2"];
            if (auto v = p2["x"].asDouble()) loadedcp.p2.x = static_cast<float>(v.unwrap());
            if (auto v = p2["y"].asDouble()) loadedcp.p2.y = static_cast<float>(v.unwrap());
            if (auto v = p2["vx"].asDouble()) loadedcp.p2.vx = static_cast<float>(v.unwrap());
            if (auto v = p2["vy"].asDouble()) loadedcp.p2.vy = static_cast<float>(v.unwrap());
            if (auto v = p2["rot"].asDouble()) loadedcp.p2.rot = static_cast<float>(v.unwrap());
            if (auto v = p2["upsideDown"].asBool()) loadedcp.p2.upsideDown = v.unwrap();
            if (auto v = p2["onGround"].asBool()) loadedcp.p2.onGround = v.unwrap();
            if (auto v = p2["speed"].asDouble()) loadedcp.p2.speed = v.unwrap();
            if (auto v = p2["speedm"].asDouble()) loadedcp.p2.speedm = v.unwrap();
            if (auto v = p2["gravity"].asDouble()) loadedcp.p2.gravity = v.unwrap();
            if (auto v = p2["gravitymod"].asDouble()) loadedcp.p2.gravitymod = static_cast<float>(v.unwrap());
            if (auto v = p2["gamemode"].asInt()) loadedcp.p2.gamemode = v.unwrap();
        }

        out.push_back(loadedcp);
    }

    return out;
}

bool CPMGR::removeCP(int levelID, std::size_t index) {
    return removeSlot(levelID, index);
}

bool CPMGR::removeNotSavedCP(std::size_t index) {
    if (index >= s_list.size()) return false;
    s_list.erase(s_list.begin() + index);
    return true;
}