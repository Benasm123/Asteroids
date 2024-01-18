#pragma once
#include "object_pool.hpp"
#include "SDL.h"
#include "SDL_image.h"
#include "ecs.hpp"
#include "components.hpp"

constexpr int SIZE_OF_ASTEROID = 64;

struct Vec2 {
    float x;
    float y;
};

struct iVec2 {
    int x;
    int y;
};

enum Wall {
    TOP,
    RIGHT,
    BOTTOM,
    LEFT
};

struct AsteroidsGame {
    SDL_Window* window;
    iVec2 window_size;

    bool update_deltatime = true;
    double deltatime;

    SDL_Renderer* renderer = nullptr;
    ECS ecs{};

    Entity player;
    Entity booster;

    ObjectPool<Entity, 200> asteroids;

    float asteroid_spawn_time = 0.1f;

    std::unordered_map<std::string, TextureID> texture_id_map;
    std::unordered_map<TextureID, SDL_Texture*> texture_map;

    AsteroidsGame();
    void InitSDL();
    void InitECS();
    void ManageEvents();
    TextureID GetTexture(const std::string& texture_path);

    void InitPlayer();
    void InitBooster();
    void InitAsteroids();
    
    template <int C>
    void InitBullets(ObjectPool<Entity, C>& bullets) {
        for (auto& bullet_object : bullets.objects) {
        bullet_object.object = ecs.CreateEntity();
        Entity bullet = bullet_object.object;

        ecs.AddComponentToEntity<Position>(bullet);
        ecs.AddComponentToEntity<Velocity>(bullet);
        ecs.AddComponentToEntity<Rotation>(bullet);

        Texture& texture = ecs.AddComponentToEntity<Texture>(bullet);
        texture.id = GetTexture("Assets/Bullet8.png");
        SDL_QueryTexture(texture_map[texture.id], nullptr, nullptr, &texture.w, &texture.h);
        texture.x_centre = texture.w / 2;
        texture.y_centre = texture.h / 2;

        auto& collider = ecs.AddComponentToEntity<BoxCollider>(bullet);
        collider.w = texture.w;
        collider.h = texture.h;

        auto& speed = ecs.AddComponentToEntity<Speed>(bullet);
        speed.value = 1000;
        
        ecs.entity_state[bullet] = SLEEP;
    };
}
};