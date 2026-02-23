#pragma once

struct PlayerState {
    float x = 0.f, y = 0.f, vx = 0.f, vy = 0.f, rot = 0.f;

    bool upsideDown = false, onGround = false;

    GameObjectType gamemode = GameObjectType::CubePortal;
};

struct CheckpointData {
    PlayerState p1{};
    bool hasP2 = false;
    PlayerState p2{};

    int perc = 0;
};