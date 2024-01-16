#define SDL_MAIN_HANDLED

#include "iostream"
#include "SDL.h"
#include "SDL_image.h"
#include <cmath>

// TODO:
//      Velocity needs to be vector and seperate from forward, as only when trusters are on do we apply to forward.
//      Asteroids need implementing
//      Collision detection
//      Player Lives

struct Vec2 {
    float x;
    float y;
};

struct Asteroid {
    SDL_Texture* texture = nullptr;

    Vec2 position;
    Vec2 velocity;
    float rotation;

    float rotation_velocity;
}

struct Player {
    SDL_Texture* texture = nullptr;
    SDL_Texture* booster = nullptr;

    Vec2 position{};
    float velocity = 0.0f;
    bool thrust = false;
    float rotation = 0;
    int rotation_velocity = 0;
    Vec2 forward = {0, -1};

    void InitTexture(SDL_Renderer* renderer) {
        texture = IMG_LoadTexture(renderer, "Assets/Player32.png");
        booster = IMG_LoadTexture(renderer, "Assets/Booster32.png");
    }

    void Update() {
        if (thrust) {
            velocity += 0.0005f;
        }
        velocity -= 0.0003f;
        if (velocity <= 0) {
            velocity = 0;
        }

        rotation += (float)rotation_velocity / 300.0;
        if (rotation > 6.283) {
            rotation -= 6.283;
        } else if (rotation < 0) {
            rotation += 6.283;
        }

        forward = {
            (float)std::sin(rotation),
            (float)-std::cos(rotation)
        };

        position.x += velocity * forward.x;
        position.y += velocity * forward.y;
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

struct Asteroids {
    SDL_Window* window;
    SDL_Renderer* renderer;

    Player player;
};

void InitSDL(Asteroids& game) {
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

int main(int argv, char** args) {
    std::cout << "Hello World!\n";

    Asteroids game{};
    InitSDL(game);

    game.player.InitTexture(game.renderer);
    game.player.position.x = 100;
    game.player.position.y = 100;

    SDL_Event event;
    while (true) {
        SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
	    SDL_RenderClear(game.renderer);

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

        game.player.Update();
        game.player.Render(game.renderer);

        SDL_RenderPresent(game.renderer);
    }

    return 0;
}