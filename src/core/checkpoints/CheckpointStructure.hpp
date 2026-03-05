#pragma once

#include <Geode/binding/GameObject.hpp>
#include <cstdint>

struct PlayerState {
    float x = 0.f, y = 0.f, vx = 0.f, vy = 0.f, rot = 0.f, gravitymod = 0.f;
    double speed = 0.0, gravity = 0.0, speedm = 0.0;
    
    bool upsideDown = false, onGround = false, mini = false;

    int gamemode = 0;
};

struct CheckpointData {
    PlayerState p1{};
    bool hasP2 = false;
    PlayerState p2{};
    
    double levelTime = 0.0;
    unsigned int musicTime = 0;
    uint64_t seed = 0;

    bool editor = false;

    int perc = 0;
};