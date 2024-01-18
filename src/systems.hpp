#pragma once

#include <cmath>

void RenderAction(struct AsteroidsGame& game, const struct System& system) {
    for (auto entity : system.entities) {
        if (game.ecs.entity_state[entity] != ACTIVE) continue;
        Texture texture = game.ecs.GetComponent<Texture>(entity);

        Position position = game.ecs.GetComponent<Position>(entity);
        Rotation rotation = game.ecs.GetComponent<Rotation>(entity);

        SDL_Rect dest;
        dest.x = position.x;
        dest.y = position.y;
        dest.w = texture.w;
        dest.h = texture.h;

        SDL_Texture* sdl_texture = game.texture_map[texture.id];

        // Dont need to keep doing this every frame!

        SDL_Point centre {
            texture.x_centre, 
            texture.y_centre
        };

        SDL_RenderCopyEx(game.renderer, sdl_texture, nullptr, &dest, rotation.value * 57.2957795f, &centre, SDL_FLIP_NONE);        
    }
};

void MoveAction(struct AsteroidsGame& game, const struct System& system) {
    for (auto entity : system.entities) {
        if (game.ecs.entity_state[entity] != ACTIVE) continue;

        Position& position = game.ecs.GetComponent<Position>(entity);
        Velocity velocity = game.ecs.GetComponent<Velocity>(entity);

        position.x += velocity.x * game.deltatime;
        position.y += velocity.y * game.deltatime;
    }
};

void RotateAction(struct AsteroidsGame& game, const struct System& system) {
    for (auto entity : system.entities) {
        if (game.ecs.entity_state[entity] != ACTIVE) continue;

        Rotation& rotation = game.ecs.GetComponent<Rotation>(entity);
        RotationVelocity velocity = game.ecs.GetComponent<RotationVelocity>(entity);

        rotation.value += velocity.value * game.deltatime;
    }
};

void EngineAction(struct AsteroidsGame& game, const struct System& system) {
    for (auto entity : system.entities) {
        if (game.ecs.entity_state[entity] != ACTIVE) continue;

        Thrust thrust = game.ecs.GetComponent<Thrust>(entity);

        Rotation rotation = game.ecs.GetComponent<Rotation>(entity);
        Velocity& velocity = game.ecs.GetComponent<Velocity>(entity);

        // Forward Direction of entity.
        float x = (float)std::sin(rotation.value);
        float y = (float)-std::cos(rotation.value);

        velocity.x += thrust.value * thrust.power * x * game.deltatime;
        velocity.y += thrust.value * thrust.power * y * game.deltatime;
    }
};

// Only works on asteroids --- TODO: Generalize
void SetInactiveOnLeaveScreenAction(struct AsteroidsGame& game, const struct System& system) {
    for (auto entity : system.entities) {
        if (game.ecs.entity_state[entity] != ACTIVE) continue;

        Position position = game.ecs.GetComponent<Position>(entity);

        if (!(position.x < -(100) || position.x > game.window_size.x + (100) ||
            position.y < -(100) || position.y > game.window_size.y + (100))) continue;
        
        game.ecs.entity_state[entity] = SLEEP;
    }
};

void CollisionDetectAction(struct AsteroidsGame& game, const struct System& system) {
    auto player_position = game.ecs.GetComponent<Position>(game.player);
    auto player_collider = game.ecs.GetComponent<BoxCollider>(game.player);
    auto player_shooter = game.ecs.GetComponent<Shooter>(game.player);

    for (auto entity : system.entities) {
        if (game.ecs.entity_state[entity] != ACTIVE) continue;

        auto& collider = game.ecs.GetComponent<BoxCollider>(entity);
        auto position = game.ecs.GetComponent<Position>(entity);

        if ((position.x + collider.x_offset <= (player_position.x + player_collider.w + player_collider.x_offset) && (position.x + collider.w + collider.x_offset) >= player_position.x + player_collider.x_offset) &&
            (position.y + collider.y_offset <= (player_position.y + player_collider.h + player_collider.y_offset) && (position.y + collider.h + collider.y_offset) >= player_position.y + player_collider.y_offset)
        ) {
            // std::cout << "Collision\n";
            game.asteroids.objects[game.asteroids.object_to_id[entity]].alive = false;
            game.ecs.entity_state[entity] = SLEEP;
        }

        // Check bullet Collisions
        for (auto bullet_item : player_shooter.bullets.objects) {
            Entity bullet = bullet_item.object;
            auto bullet_position = game.ecs.GetComponent<Position>(bullet);
            auto bullet_collider = game.ecs.GetComponent<BoxCollider>(bullet);

            if ((position.x + collider.x_offset <= (bullet_position.x + bullet_collider.w + bullet_collider.x_offset) && (position.x + collider.w + collider.x_offset) >= bullet_position.x + bullet_collider.x_offset) &&
            (position.y + collider.y_offset <= (bullet_position.y + bullet_collider.h + bullet_collider.y_offset) && (position.y + collider.h + collider.y_offset) >= bullet_position.y + bullet_collider.y_offset)
            ) {
                // std::cout << "Collision\n";
                game.asteroids.objects[game.asteroids.object_to_id[entity]].alive = false;
                game.ecs.entity_state[entity] = SLEEP;
            }
        }
    }
}

void FollowAction(struct AsteroidsGame& game, const struct System& system) {
    for (auto entity : system.entities) {
        if (game.ecs.entity_state[entity] != ACTIVE) continue;

        auto& position = game.ecs.GetComponent<Position>(entity);
        auto& rotation = game.ecs.GetComponent<Rotation>(entity);
        auto follow = game.ecs.GetComponent<Follow>(entity);

        auto follow_position = game.ecs.GetComponent<Position>(follow.to_follow);
        auto follow_rotation = game.ecs.GetComponent<Rotation>(follow.to_follow);

        position.x = follow_position.x + follow.x_offset;
        position.y = follow_position.y + follow.y_offset;

        rotation.value = follow_rotation.value;
    }
}

void DragAction(struct AsteroidsGame& game, const struct System& system) {
    for (auto entity : system.entities) {
        if (game.ecs.entity_state[entity] != ACTIVE) continue;

        auto& velocity = game.ecs.GetComponent<Velocity>(entity);
        auto& drag_coefficient = game.ecs.GetComponent<DragCoefficient>(entity);

        velocity.x -= velocity.x * drag_coefficient.value * game.deltatime;
        velocity.y -= velocity.y * drag_coefficient.value * game.deltatime;
    }
}

void ShootBullet(struct AsteroidsGame& game, const Entity bullet, const Position from, const Rotation wanted_rotation) {
    Position& position = game.ecs.GetComponent<Position>(bullet);
    Velocity& velocity = game.ecs.GetComponent<Velocity>(bullet);
    Rotation& rotation = game.ecs.GetComponent<Rotation>(bullet);
    Texture texture = game.ecs.GetComponent<Texture>(bullet);
    Speed speed = game.ecs.GetComponent<Speed>(bullet);

    position.x = from.x - (texture.w / 2);
    position.y = from.y - (texture.h / 2);
    rotation = wanted_rotation;
    
    float direction_x = (float)std::sin(rotation.value);
    float direction_y = (float)-std::cos(rotation.value);

    velocity.x = direction_x * speed.value;
    velocity.y = direction_y * speed.value;
}

void ShooterAction(struct AsteroidsGame& game, const struct System& system) {
    for (auto entity : system.entities) {
        Shooter& shooter = game.ecs.GetComponent<Shooter>(entity);
        if (shooter.cooldown > 0.0) {
            shooter.cooldown -= game.deltatime;
        }
        else
        {
            if (shooter.shooting) {
                auto& bullet_item = shooter.bullets.ObtainObject();
                bullet_item.alive = true;

                Entity bullet = bullet_item.object;

                game.ecs.entity_state[bullet] = ACTIVE;

                Position position = game.ecs.GetComponent<Position>(entity);
                Texture texture = game.ecs.GetComponent<Texture>(entity);
                Rotation rotation = game.ecs.GetComponent<Rotation>(entity);

                float direction_x = (float)std::sin(rotation.value);
                float direction_y = (float)-std::cos(rotation.value);

                Position from{};
                from.x = position.x + (texture.w / 2.0) + (direction_x * (texture.w / 2.0)); 
                from.y = position.y + (texture.h / 2.0) + (direction_y * (texture.h / 2.0));

                ShootBullet(game, bullet, from, rotation);

                shooter.cooldown = shooter.fire_rate;
            }
        }
    }
}