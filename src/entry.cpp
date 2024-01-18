#define SDL_MAIN_HANDLED

#include "iostream"
#include <chrono>
#include "game.hpp"

void AsteroidSpawnAction(struct AsteroidsGame& game, Entity entity) {
    Position& position = game.ecs.GetComponent<Position>(entity);
    Velocity& velocity = game.ecs.GetComponent<Velocity>(entity);

    game.ecs.entity_state[entity] = ACTIVE;

    int val = std::rand();
    while (val > 4) {
        val = std::rand() / ((RAND_MAX + 1u) / 4);
    }
    Wall wall = (Wall)val;

    switch (wall) {
        case TOP:
            position.y = -SIZE_OF_ASTEROID;
            position.x = std::rand();
            while (position.x > (game.window_size.x + 200)) {
                position.x = std::rand() / ((RAND_MAX + 1u) / (game.window_size.x + 200));
            }
            position.x -= 100;
            break;
        case BOTTOM:
            position.y = game.window_size.y + SIZE_OF_ASTEROID;
            position.x = std::rand();
            while (position.x > (game.window_size.x + 200)) {
                position.x = std::rand() / ((RAND_MAX + 1u) / (game.window_size.x + 200));
            }
            position.x -= 100;
            break;
        case RIGHT:
            position.x = game.window_size.x + SIZE_OF_ASTEROID;
            position.y = std::rand();
            while (position.y > (game.window_size.y + 200)) {
                position.y = std::rand() / ((RAND_MAX + 1u) / (game.window_size.y + 200));
            }
            position.x -= 100;
            break;
        case LEFT:
            position.x = -SIZE_OF_ASTEROID;
            position.y = std::rand();
            while (position.y > (game.window_size.y + 200)) {
                position.y = std::rand() / ((RAND_MAX + 1u) / (game.window_size.y + 200));
            }
            position.y -= 100;
            break;
    }

    // Choose a random position off screen.

    int random_x = std::rand();
    while (random_x > game.window_size.x) {
        random_x = std::rand() / ((RAND_MAX + 1u) / game.window_size.x);
    }

    int random_y = std::rand();
    while (random_y > game.window_size.y) {
        random_y = std::rand() / ((RAND_MAX + 1u) / game.window_size.y);
    }

    // Needs to travel to (random point on screen - position) Can use this to get the speed too.
    velocity.x = (random_x - position.x) * 0.2f;
    velocity.y = (random_y - position.y) * 0.2f;
}

// TODO:
//      Velocity needs to be vector and seperate from forward, as only when trusters are on do we apply to forward.
//      Asteroids need implementing
//      Collision detection
//      Player Lives

int main(int argv, char** args) {
    AsteroidsGame game{};

    SDL_GetWindowSize(game.window, &game.window_size.x, &game.window_size.y);

    // Deltatime
    game.deltatime = 0;
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
        if (game.update_deltatime) game.deltatime = diff.count();
        last_frame = current_frame;

        frame_count++;
        dt_sum += game.deltatime;

        fps_cooldown -= game.deltatime; 
        if (fps_cooldown < 0) {
            fps_cooldown = 1.0;
            std::cout << "FPS: " << 1 / (dt_sum / (double)frame_count) << "\n";
            frame_count = 0; 
            dt_sum = 0.0;
        }

        SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
	    SDL_RenderClear(game.renderer);

        // game.update?
        game.ManageEvents();
        game.ecs.Update(game);

        // This is not optimal... Need to figure out how to tell object pool when objects die in a general way. Can just avoid using object pools tbf and just preallocate entity space for objects and components;
        auto& shooter = game.ecs.GetComponent<Shooter>(game.player);
        for (auto& bullet : shooter.bullets.objects) {
            if (game.ecs.entity_state[bullet.object] != ACTIVE) bullet.alive = false;
        }

        for (auto& asteroid : game.asteroids.objects) {
            if (game.ecs.entity_state[asteroid.object] != ACTIVE) asteroid.alive = false;
        }

        asteroid_cooldown -= game.deltatime;
        if (asteroid_cooldown < 0) {
            asteroid_cooldown = game.asteroid_spawn_time;

            auto& asteroid_to_enable = game.asteroids.ObtainObject();

            AsteroidSpawnAction(game, asteroid_to_enable.object);

            game.asteroids.objects[game.asteroids.object_to_id[asteroid_to_enable.object]].alive = true;
        }

        SDL_RenderPresent(game.renderer);
    }

    return 0;
}