#pragma once
#include "ecs.hpp"

struct Position : Component {
    float x{};
    float y{};
};

struct Velocity : Component {
    float x{};
    float y{};
};

struct Rotation : Component {
    float value{};
};

struct RotationVelocity : Component {
    float value{};
};

struct Thrust : Component {
    float power = 900;
    float value; // Allows variable thrust values for controller.
};

// typedef unsigned int Texture;
typedef unsigned int TextureID;

struct Texture : Component {
    unsigned int id{};
    int w{};
    int h{};
    int x_centre{};
    int y_centre{};
};

struct BoxCollider : Component {
    float w{};
    float h{};
    float x_offset{};
    float y_offset{};
};

struct CollideWithPlayer : Component {
    bool test{};
};

// Can add some check to make sure entities have all required components for other components
// to_follow needs position, rotation
struct Follow : Component {
    Entity to_follow{};
    int x_offset{};
    int y_offset{};
};

struct DragCoefficient : Component {
    float value{0.99};
};

struct Shooter : Component {
    ObjectPool<Entity, 200> bullets;
    float fire_rate{0.1};
    float cooldown{0.0};
    bool shooting{false};
};

struct Speed : Component {
    float value;
};