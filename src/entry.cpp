#define SDL_MAIN_HANDLED

#include "iostream"
#include "SDL.h"
#include "SDL_image.h"
#include <cmath>
#include <chrono>
#include "object_pool.hpp"
#include <cstdlib>
#include "ecs.hpp"

// TODO:
//      Velocity needs to be vector and seperate from forward, as only when trusters are on do we apply to forward.
//      Asteroids need implementing
//      Collision detection
//      Player Lives

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

// COMPONENT DEFINITIONS
struct Position : Component {
    Vec2 value;
};

struct Velocity : Component {
    Vec2 value;
};

struct Rotation : Component {
    float value;
};

struct Texture : Component {
    SDL_Texture* texture = nullptr;
};
// END COMPONENT DEFINITIONS

// SYSTEM DEFINITIONS
struct Move : System {

};
// END SYSTEM DEFINITIONS

struct Asteroid {
    SDL_Texture* texture = nullptr;

    Vec2 position;
    Vec2 velocity;
    float rotation;

    float rotation_velocity;

    void InitTexture(SDL_Renderer* renderer) {
        texture = IMG_LoadTexture(renderer, "Assets/Asteroid64.png");
    }    

    void Prepare(iVec2 window_dimensions) {
        int val = std::rand();
        while (val > 4) {
            val = std::rand() / ((RAND_MAX + 1u) / 4);
        }
        Wall wall = (Wall)val;

        switch (wall) {
            case TOP:
                position.y = -SIZE_OF_ASTEROID;
                position.x = std::rand();
                while (position.x > (window_dimensions.x + 200)) {
                    position.x = std::rand() / ((RAND_MAX + 1u) / (window_dimensions.x + 200));
                }
                position.x -= 100;
                break;
            case BOTTOM:
                position.y = window_dimensions.y + SIZE_OF_ASTEROID;
                position.x = std::rand();
                while (position.x > (window_dimensions.x + 200)) {
                    position.x = std::rand() / ((RAND_MAX + 1u) / (window_dimensions.x + 200));
                }
                position.x -= 100;
                break;
            case RIGHT:
                position.x = window_dimensions.x + SIZE_OF_ASTEROID;
                position.y = std::rand();
                while (position.y > (window_dimensions.y + 200)) {
                    position.y = std::rand() / ((RAND_MAX + 1u) / (window_dimensions.y + 200));
                }
                position.x -= 100;
                break;
            case LEFT:
                position.x = -SIZE_OF_ASTEROID;
                position.y = std::rand();
                while (position.y > (window_dimensions.y + 200)) {
                    position.y = std::rand() / ((RAND_MAX + 1u) / (window_dimensions.y + 200));
                }
                position.y -= 100;
                break;
        }

        // Choose a random position off screen.
        // position = {
        //     window_dimensions.x / 2.0f,
        //     window_dimensions.y / 2.0f
        // };

        int random_x = std::rand();
        while (random_x > window_dimensions.x) {
            random_x = std::rand() / ((RAND_MAX + 1u) / window_dimensions.x);
        }

        int random_y = std::rand();
        while (random_y > window_dimensions.y) {
            random_y = std::rand() / ((RAND_MAX + 1u) / window_dimensions.y);
        }

        // Needs to travel to (random point on screen - position) Can use this to get the speed too.
        velocity = {
            (random_x - position.x) * 0.4f,
            (random_y - position.y) * 0.4f
        };

        // Some random rotation / can make them rotate as they travel too?
        rotation = 0;
    }

    bool Update(double dt, iVec2 window_dimensions)  {
        position.x += velocity.x * dt;
        position.y += velocity.y * dt;

        if (position.x < -SIZE_OF_ASTEROID || position.x > window_dimensions.x + SIZE_OF_ASTEROID) return false;
        if (position.y < -SIZE_OF_ASTEROID || position.y > window_dimensions.y + SIZE_OF_ASTEROID) return false;
        return true;
    }

    void Render(SDL_Renderer* renderer) { 
        SDL_Rect dest;
        dest.x = position.x;
        dest.y = position.y;

        SDL_QueryTexture(texture, nullptr, nullptr, &dest.w, &dest.h);

        SDL_Point centre {
            (dest.w) / 2, 
            (dest.h) / 2
        };

        SDL_RenderCopyEx(renderer, texture, nullptr, &dest, rotation * 57.2957795f, &centre, SDL_FLIP_NONE);
    }
};

struct Player {
    // Control Settings
    float speed = 900.0f;
    float rotation_speed = 10.0f;

    // Textures
    SDL_Texture* texture = nullptr;
    SDL_Texture* booster = nullptr;

    // Components
    Vec2 position{};
    Vec2 velocity = {};
    Vec2 forward = {0, -1};
    bool thrust = false;
    float rotation = 0;
    int rotation_velocity = 0;

    void InitTexture(SDL_Renderer* renderer) {
        texture = IMG_LoadTexture(renderer, "Assets/Player32.png");
        booster = IMG_LoadTexture(renderer, "Assets/Booster32.png");
    }    

    void Update(double dt) {
        forward = {
            (float)std::sin(rotation),
            (float)-std::cos(rotation)
        };

        if (thrust) {
            velocity.x += forward.x * speed * dt;
            velocity.y += forward.y * speed * dt;
        }

        velocity.x -= (velocity.x * 0.99) * dt;
        velocity.y -= (velocity.y * 0.99) * dt;

        rotation += (float)rotation_velocity * rotation_speed * dt;
        if (rotation > 6.283) {
            rotation -= 6.283;
        } else if (rotation < 0) {
            rotation += 6.283;
        }

        position.x += velocity.x * dt;
        position.y += velocity.y * dt;
    }

    void Render(SDL_Renderer* renderer) {
        SDL_Rect dest;
        dest.x = position.x;
        dest.y = position.y;

        SDL_QueryTexture(texture, nullptr, nullptr, &dest.w, &dest.h);

        SDL_Point centre {
            (dest.w) / 2, 
            (dest.h) / 2
        };

        SDL_RenderCopyEx(renderer, texture, nullptr, &dest, rotation * 57.2957795f, &centre, SDL_FLIP_NONE);

        if (!thrust) return;

        SDL_Rect booster_pos;
        booster_pos.x = position.x;
        booster_pos.y = position.y + dest.h;

        SDL_QueryTexture(booster, nullptr, nullptr, &booster_pos.w, &booster_pos.h);

        SDL_Point booster_centre {
            (dest.w) / 2, 
            - (dest.h) / 2
        };

        SDL_RenderCopyEx(renderer, booster, nullptr, &booster_pos, rotation * 57.2957795f, &booster_centre, SDL_FLIP_NONE);
    }
};

struct AsteroidsGame {
    SDL_Window* window;
    iVec2 window_size;

    SDL_Renderer* renderer;

    Player player;

    ObjectPool<Asteroid, 20> asteroids;

    float asteroid_spawn_time = 1.0f;
};

void InitSDL(AsteroidsGame& game) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    game.window = SDL_CreateWindow(
        "Asteroids",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        1000, 800,
        SDL_WINDOW_RESIZABLE
    );

    game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
}

void manage_events(AsteroidsGame& game);

int main(int argv, char** args) {
    AsteroidsGame game{};

    InitSDL(game);

    for (auto& asteroid : game.asteroids.objects) {
        asteroid.object.InitTexture(game.renderer);
    }

    game.player.InitTexture(game.renderer);
    game.player.position.x = 100;
    game.player.position.y = 100;

    SDL_GetWindowSize(game.window, &game.window_size.x, &game.window_size.y);

    // Deltatime
    double dt = 0;
    auto last_frame = std::chrono::steady_clock::now();

    // Manage Spawning Asteroids
    double asteroid_cooldown = game.asteroid_spawn_time;

    // For printing FPS
    double fps_cooldown = 1.0;
    int frame_count = 0;
    double dt_sum = 0.0;

    while (true) {
        // Get window size every frame (Can do in SDL Resize event thought)
        SDL_GetWindowSize(game.window, &game.window_size.x, &game.window_size.y);

        // DT Calc
        auto current_frame = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff{current_frame - last_frame};
        dt = diff.count();
        last_frame = current_frame;

        frame_count++;
        dt_sum += dt;

        fps_cooldown -= dt; 
        if (fps_cooldown < 0) {
            fps_cooldown = 1.0;
            std::cout << "FPS: " << 1 / (dt_sum / (double)frame_count) << "\n";
            frame_count = 0; 
            dt_sum = 0.0;
        }

        SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
	    SDL_RenderClear(game.renderer);

        manage_events(game);

        game.player.Update(dt);
        game.player.Render(game.renderer);

        asteroid_cooldown -= dt;
        if (asteroid_cooldown < 0) {
            asteroid_cooldown = game.asteroid_spawn_time;

            auto& asteroid_to_enable = game.asteroids.ObtainObject();
            asteroid_to_enable.alive = true;
            asteroid_to_enable.object.Prepare(game.window_size);
        }

        for (auto& asteroid : game.asteroids.objects) {
            if (!asteroid.alive) continue;
            if (!asteroid.object.Update(dt, game.window_size)) asteroid.alive = false;
            asteroid.object.Render(game.renderer); 
        }

        SDL_RenderPresent(game.renderer);
    }

    return 0;
}

void manage_events(AsteroidsGame& game) {
    SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    exit(0);
                    break;
                case SDL_KEYDOWN:
                    if (event.key.repeat != 0) continue;
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_W:
                            game.player.thrust = true;
                            break;
                        // case SDL_SCANCODE_S:
                        //     game.player.velocity.y += 1;
                        //     break;
                        case SDL_SCANCODE_A:
                            game.player.rotation_velocity -= 1;
                            break;
                        case SDL_SCANCODE_D:
                            game.player.rotation_velocity += 1;
                            break;
                        default:
                            break;
                    }
                    break;

                case SDL_KEYUP:
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_W:
                            game.player.thrust = false;
                            break;
                        // case SDL_SCANCODE_S:
                        //     game.player.velocity.y -= 1;
                        //     break;
                        case SDL_SCANCODE_A:
                            game.player.rotation_velocity += 1;
                            break;
                        case SDL_SCANCODE_D:
                            game.player.rotation_velocity -= 1;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
}