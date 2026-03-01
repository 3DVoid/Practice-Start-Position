#pragma once

#include <Geode/binding/GameObject.hpp>

struct PlayerState {
    float x = 0.f, y = 0.f, vx = 0.f, vy = 0.f, rot = 0.f, gravitymod = 0.f;
    double speed = 0.0, gravity = 0.0, speedm = 0.0;
    
    bool upsideDown = false, onGround = false;

    int gamemode = 0;
};

struct CheckpointData {
    PlayerState p1{};
    bool hasP2 = false;
    PlayerState p2{};
    
    
    int perc = 0;
};