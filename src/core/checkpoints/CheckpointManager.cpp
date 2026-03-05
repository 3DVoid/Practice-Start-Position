#include "CheckpointManager.hpp"
#include <Geode/Geode.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <cstdint>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

namespace {
    std::vector<CheckpointData> s_list;
    std::optional<CheckpointData> s_loaded;

    std::filesystem::path getCheckpointPath(int levelID, bool editor) {
        if (editor) {
            auto dir = geode::Mod::get()->getSaveDir() / "saved-editor-checkpoints";
            std::filesystem::create_directories(dir);
            return dir / (std::to_string(levelID) + ".json");
        }
        else {
            auto dir = geode::Mod::get()->getSaveDir() / "saved-checkpoints";
            std::filesystem::create_directories(dir);
            return dir / (std::to_string(levelID) + ".json");
        }
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
            { "gravitymod", ps.gravitymod },
            { "mini", ps.mini }
        });
    }

    matjson::Value checkpointToJson(std::size_t index, CheckpointData const& cp) {
        std::string sn = "slot" + std::to_string(index);
        return matjson::makeObject({
            { "name", sn },
            { "editor", cp.editor },
            { "perc", cp.perc },
            { "hasP2", cp.hasP2 },
            { "p1", playerStateToJson(cp.p1) },
            { "p2", cp.hasP2 ? playerStateToJson(cp.p2) : matjson::Value()},
            { "levelTime", cp.levelTime },
            { "musicTime", cp.musicTime },
            { "seed", cp.seed }
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
    
    bool removeSlot(int levelID, int index, bool editor) {
        auto file = getCheckpointPath(levelID, editor);
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

std::optional<CheckpointData> CPMGR::getCP(std::size_t index) {
    if (index >= s_list.size()) {
        return std::nullopt;
    }
    return s_list[index];
}

bool CPMGR::saveCP(int levelID, std::size_t index, bool editor) {
    auto cp = getCP(index);
    if (!cp.has_value()) {
        return false;
    }

    auto file = getCheckpointPath(levelID, editor);
    auto json = readOrCreateJSON(file, levelID);
    if(!addSlot(json, cp.value())) return false;
    return writeJson(file, json);
}

bool s_fromLoad = false;
bool s_doCheckpoint = false;

bool CPMGR::loadCP(int levelID, std::size_t index, bool editor) {
    auto file = getCheckpointPath(levelID, editor);

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
    if (auto v = p1["x"].as<float>()) loadedcp.p1.x = (v.unwrap());
    if (auto v = p1["y"].as<float>()) loadedcp.p1.y = (v.unwrap());
    if (auto v = p1["vx"].as<float>()) loadedcp.p1.vx = (v.unwrap());
    if (auto v = p1["vy"].as<float>()) loadedcp.p1.vy = (v.unwrap());
    if (auto v = p1["rot"].as<float>()) loadedcp.p1.rot = (v.unwrap());
    if (auto v = p1["upsideDown"].asBool()) loadedcp.p1.upsideDown = v.unwrap();
    if (auto v = p1["onGround"].asBool()) loadedcp.p1.onGround = v.unwrap();
    if (auto v = p1["speed"].asDouble()) loadedcp.p1.speed = v.unwrap();
    if (auto v = p1["speedm"].asDouble()) loadedcp.p1.speedm = v.unwrap();
    if (auto v = p1["gravity"].asDouble()) loadedcp.p1.gravity = v.unwrap();
    if (auto v = p1["gravitymod"].as<float>()) loadedcp.p1.gravitymod = (v.unwrap());
    if (auto v = p1["gamemode"].asInt()) loadedcp.p1.gamemode = v.unwrap();
    if (auto v = p1["mini"].asBool()) loadedcp.p1.mini = v.unwrap();

    if (loadedcp.hasP2 && slot["p2"].isObject()) {
        auto const& p2 = slot["p2"];
        if (auto v = p2["x"].as<float>()) loadedcp.p2.x = (v.unwrap());
        if (auto v = p2["y"].as<float>()) loadedcp.p2.y = (v.unwrap());
        if (auto v = p2["vx"].as<float>()) loadedcp.p2.vx = (v.unwrap());
        if (auto v = p2["vy"].as<float>()) loadedcp.p2.vy = (v.unwrap());
        if (auto v = p2["rot"].as<float>()) loadedcp.p2.rot = (v.unwrap());
        if (auto v = p2["upsideDown"].asBool()) loadedcp.p2.upsideDown = v.unwrap();
        if (auto v = p2["onGround"].asBool()) loadedcp.p2.onGround = v.unwrap();
        if (auto v = p2["speed"].asDouble()) loadedcp.p2.speed = v.unwrap();
        if (auto v = p2["speedm"].asDouble()) loadedcp.p2.speedm = v.unwrap();
        if (auto v = p2["gravity"].asDouble()) loadedcp.p2.gravity = v.unwrap();
        if (auto v = p2["gravitymod"].as<float>()) loadedcp.p2.gravitymod = (v.unwrap());
        if (auto v = p2["gamemode"].asInt()) loadedcp.p2.gamemode = v.unwrap();
        if (auto v = p2["mini"].asBool()) loadedcp.p2.mini = v.unwrap();
    }

    if (auto v = slot["levelTime"].asDouble()) loadedcp.levelTime = v.unwrap();
    if (auto v = slot["musicTime"].asInt()) loadedcp.musicTime = v.unwrap();
    if (auto v = slot["seed"].as<uint64_t>()) loadedcp.seed = v.unwrap();
    if (auto v = slot["editor"].asBool()) loadedcp.editor = v.unwrap();

    s_loaded = loadedcp;

    auto pl = PlayLayer::get();
    if (!pl) {
        geode::log::error("Unable To Auto-Apply, No PlayLayer Found. Please Report To 3dvoidyt (Discord or GD) If Error Consists.");
        return false;
    } 

    return applyCP(pl);
    // return applyCP(pl);
}

bool CPMGR::applyCP(PlayLayer* pl) {
    if (!pl) return false;
    if (!s_loaded) return false;
    if (!pl->m_player1) return false;

    auto const& cp = s_loaded.value();

    pl->toggleDualMode(nullptr, cp.hasP2, pl->m_player1, true);
    pl->togglePracticeMode(false);
    pl->togglePracticeMode(true);

    // if (cp.hasP2 && pl->m_player2) pl->m_player2->switchedToMode(modeToPortal(cp.p2.gamemode));

    geode::log::info("Applying CP: x={}, y={}, perc={}", cp.p1.x, cp.p1.y, cp.perc);

    pl->resetLevel();

    pl->m_player1->switchedToMode(static_cast<GameObjectType>(cp.p1.gamemode));
    pl->m_player1->flipGravity(cp.p1.upsideDown, true);

    pl->m_player1->updateCheckpointMode(false);

    pl->m_player1->setRotation(cp.p1.rot);
    pl->m_player1->m_isOnGround = cp.p1.onGround;
    pl->m_player1->m_playerSpeed = cp.p1.speed;
    pl->m_player1->m_speedMultiplier = cp.p1.speedm;
    pl->m_player1->m_gravity = cp.p1.gravity;
    pl->m_player1->m_gravityMod = cp.p1.gravitymod;

    pl->m_player1->togglePlayerScale(cp.p1.mini, true);
    pl->m_player1->updatePlayerScale();

    pl->m_player1->updateRotation(0.f);
    pl->m_player1->m_lastPosition = cocos2d::CCPoint{cp.p1.x, cp.p1.y};
    geode::log::info("Before setPosition: {}, {}", pl->m_player1->getPositionX(), pl->m_player1->getPositionY());
    pl->m_player1->m_position = cocos2d::CCPoint{cp.p1.x, cp.p1.y };
    geode::log::info("After setPosition: {}, {}", pl->m_player1->getPositionX(), pl->m_player1->getPositionY());
    pl->m_player1->setPosition({ cp.p1.x, cp.p1.y });

    pl->m_player1->setYVelocity(cp.p1.vy, 0);
    pl->m_player1->updateStateVariables();
    if (pl->m_isPlatformer) {
        pl->m_player1->m_platformerXVelocity = cp.p1.vx;
    }

    if (cp.hasP2 && pl->m_player2 != nullptr) {
        pl->m_player2->switchedToMode(static_cast<GameObjectType>(cp.p2.gamemode));
        pl->m_player2->flipGravity(cp.p2.upsideDown, true);

        pl->m_player2->updateCheckpointMode(false);

        pl->m_player2->setRotation(cp.p2.rot);
        pl->m_player2->m_isOnGround = cp.p2.onGround;
        pl->m_player2->m_playerSpeed = cp.p2.speed;
        pl->m_player2->m_speedMultiplier = cp.p2.speedm;
        pl->m_player2->m_gravity = cp.p2.gravity;
        pl->m_player2->m_gravityMod = cp.p2.gravitymod;

        pl->m_player2->togglePlayerScale(cp.p2.mini, true);
        pl->m_player2->updatePlayerScale();

        pl->m_player2->updateRotation(0.f);
        pl->m_player2->m_lastPosition = cocos2d::CCPoint{cp.p2.x, cp.p2.y};
        pl->m_player2->m_position = cocos2d::CCPoint{cp.p2.x, cp.p2.y };
        pl->m_player2->setPosition({ cp.p2.x, cp.p2.y });
        pl->m_player2->setYVelocity(cp.p2.vy, 0);
        pl->m_player2->updateStateVariables();
        if (pl->m_isPlatformer) {
            pl->m_player2->m_platformerXVelocity = cp.p2.vx;
        }
    }
    pl->updateCamera(0.f);

    geode::Loader::get()->queueInMainThread([pl, cp]() {
        pl->m_gameState.m_levelTime = cp.levelTime;
        pl->m_timePlayed = cp.levelTime;
        pl->m_currentTime = cp.levelTime;

        FMODAudioEngine::sharedEngine()->setMusicTimeMS(cp.musicTime, true, 0);
    });
    geode::log::info("After AFTER setPosition: {}, {}", pl->m_player1->getPositionX(), pl->m_player1->getPositionY());
    geode::log::info("m_player1 ptr in applyCP: {}", (void*)pl->m_player1);
    
    s_doCheckpoint = true;
    return true;
}

void CPMGR::clearList() {
    s_list.clear();
    s_loaded = std::nullopt;
}

std::vector<CheckpointData> CPMGR::getSavedSlots(int levelID, bool editor) {
    std::vector<CheckpointData> out;

    auto file = getCheckpointPath(levelID, editor);
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
        if (auto v = p1["x"].as<float>()) loadedcp.p1.x = (v.unwrap());
        if (auto v = p1["y"].as<float>()) loadedcp.p1.y = (v.unwrap());
        if (auto v = p1["vx"].as<float>()) loadedcp.p1.vx = (v.unwrap());
        if (auto v = p1["vy"].as<float>()) loadedcp.p1.vy = (v.unwrap());
        if (auto v = p1["rot"].as<float>()) loadedcp.p1.rot = (v.unwrap());
        if (auto v = p1["upsideDown"].asBool()) loadedcp.p1.upsideDown = v.unwrap();
        if (auto v = p1["onGround"].asBool()) loadedcp.p1.onGround = v.unwrap();
        if (auto v = p1["speed"].asDouble()) loadedcp.p1.speed = v.unwrap();
        if (auto v = p1["speedm"].asDouble()) loadedcp.p1.speedm = v.unwrap();
        if (auto v = p1["gravity"].asDouble()) loadedcp.p1.gravity = v.unwrap();
        if (auto v = p1["gravitymod"].as<float>()) loadedcp.p1.gravitymod = (v.unwrap());
        if (auto v = p1["gamemode"].asInt()) loadedcp.p1.gamemode = v.unwrap();
        if (auto v = p1["mini"].asBool()) loadedcp.p1.mini = v.unwrap();

        if (loadedcp.hasP2 && slot["p2"].isObject()) {
            auto const& p2 = slot["p2"];
            if (auto v = p2["x"].as<float>()) loadedcp.p2.x = v.unwrap();
            if (auto v = p2["y"].as<float>()) loadedcp.p2.y = v.unwrap();
            if (auto v = p2["vx"].as<float>()) loadedcp.p2.vx = v.unwrap();
            if (auto v = p2["vy"].as<float>()) loadedcp.p2.vy = v.unwrap();
            if (auto v = p2["rot"].as<float>()) loadedcp.p2.rot = v.unwrap();
            if (auto v = p2["upsideDown"].asBool()) loadedcp.p2.upsideDown = v.unwrap();
            if (auto v = p2["onGround"].asBool()) loadedcp.p2.onGround = v.unwrap();
            if (auto v = p2["speed"].asDouble()) loadedcp.p2.speed = v.unwrap();
            if (auto v = p2["speedm"].asDouble()) loadedcp.p2.speedm = v.unwrap();
            if (auto v = p2["gravity"].asDouble()) loadedcp.p2.gravity = v.unwrap();
            if (auto v = p2["gravitymod"].as<float>()) loadedcp.p2.gravitymod = v.unwrap();
            if (auto v = p2["gamemode"].asInt()) loadedcp.p2.gamemode = v.unwrap();
            if (auto v = p2["mini"].asBool()) loadedcp.p2.mini = v.unwrap();
        }

        if (auto v = slot["levelTime"].asDouble()) loadedcp.levelTime = v.unwrap();
        if (auto v = slot["musicTime"].asInt()) loadedcp.musicTime = v.unwrap();
        if (auto v = slot["seed"].as<uint64_t>()) loadedcp.seed = v.unwrap();
        if (auto v = slot["editor"].asBool()) loadedcp.editor = v.unwrap();

        out.push_back(loadedcp);
    }

    return out;
}

bool CPMGR::removeCP(int levelID, std::size_t index, bool editor) {
    return removeSlot(levelID, index, editor);
}

bool CPMGR::removeNotSavedCP(std::size_t index) {
    if (index >= s_list.size()) return false;
    s_list.erase(s_list.begin() + index);
    return true;
}