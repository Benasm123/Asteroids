#include "game.hpp"
#include "components.hpp"
#include "systems.hpp"
#include <cstdlib>

AsteroidsGame::AsteroidsGame() {
    InitSDL();
    InitECS();
    asteroids.InitMap(); // Need to do this after ECS as ECS initialises the entities which are just ids.

    InitPlayer();
    InitBooster();
    InitAsteroids();
}

void AsteroidsGame::InitSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    window = SDL_CreateWindow(
        "Asteroids",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        1000, 800,
        SDL_WINDOW_RESIZABLE
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void AsteroidsGame::InitPlayer() {
    player = ecs.CreateEntity();
    ecs.AddComponentToEntity<DragCoefficient>(player);
    auto& player_pos = ecs.AddComponentToEntity<Position>(player);
    player_pos.x = 200;
    player_pos.y = 200;

    auto& player_tex = ecs.AddComponentToEntity<Texture>(player);
    player_tex.id = GetTexture("Assets/Player32.png");
    SDL_QueryTexture(texture_map[player_tex.id], nullptr, nullptr, &player_tex.w, &player_tex.h);
    player_tex.x_centre = player_tex.w / 2;
    player_tex.y_centre = player_tex.h / 2;

    ecs.AddComponentToEntity<Velocity>(player);
    ecs.AddComponentToEntity<Thrust>(player);
    ecs.AddComponentToEntity<Rotation>(player);    
    ecs.AddComponentToEntity<RotationVelocity>(player);
    auto& player_collider = ecs.AddComponentToEntity<BoxCollider>(player);
    player_collider.w = player_tex.w - 10.0;
    player_collider.h = player_tex.h - 10.0;

    player_collider.x_offset = 5.0;
    player_collider.y_offset = 5.0;

    auto& shooter = ecs.AddComponentToEntity<Shooter>(player);
    InitBullets(shooter.bullets);
}

void AsteroidsGame::InitBooster() {

    auto player_tex_val = ecs.GetComponent<Texture>(player);

    booster = ecs.CreateEntity();
    ecs.AddComponentToEntity<Position>(booster);
    ecs.AddComponentToEntity<Rotation>(booster);

    auto& follow = ecs.AddComponentToEntity<Follow>(booster);
    follow.to_follow = player;
    follow.x_offset = 0;
    follow.y_offset = player_tex_val.h;
    
    
    auto& booster_texture = ecs.AddComponentToEntity<Texture>(booster);
    booster_texture.id = GetTexture("Assets/Booster32.png");
    std::cout << player_tex_val.id << ":" << player_tex_val.h << "\n";
    SDL_QueryTexture(texture_map[booster_texture.id], nullptr, nullptr, &booster_texture.w, &booster_texture.h);
    booster_texture.x_centre = player_tex_val.w / 2.0;
    booster_texture.y_centre = -player_tex_val.h / 2.0;

    // Disable on start
    ecs.entity_state[booster] = SLEEP;
}

void AsteroidsGame::InitAsteroids() {
    for (auto& asteroid : asteroids.objects) {
        asteroid.object = ecs.CreateEntity();

        Position& asteroid_pos = ecs.AddComponentToEntity<Position>(asteroid.object);

        ecs.AddComponentToEntity<Velocity>(asteroid.object);
        ecs.AddComponentToEntity<Rotation>(asteroid.object);

        Texture& asteroid_texture = ecs.AddComponentToEntity<Texture>(asteroid.object);
        asteroid_texture.id = GetTexture("Assets/Asteroid64.png");
        SDL_QueryTexture(texture_map[asteroid_texture.id], nullptr, nullptr, &asteroid_texture.w, &asteroid_texture.h);

        int random_number = std::rand();
        while (random_number > 40) {
            random_number = std::rand() / ((RAND_MAX + 1u) / 40);
        }
        random_number -= 20;

        float scale = 1.0 + ((float)random_number / 100.0);

        asteroid_texture.w *= scale;
        asteroid_texture.h *= scale;

        asteroid_texture.x_centre = asteroid_texture.w / 2;
        asteroid_texture.y_centre = asteroid_texture.h / 2;

        ecs.entity_state[asteroid.object] = SLEEP;

        auto& asteroid_collider = ecs.AddComponentToEntity<BoxCollider>(asteroid.object);
        asteroid_collider.w = asteroid_texture.w;
        asteroid_collider.h = asteroid_texture.h;

        ecs.AddComponentToEntity<CollideWithPlayer>(asteroid.object);
    }
}

void AsteroidsGame::InitECS() {
    ecs.components.reserve(6);
    ecs.RegisterComponent<Position>();
    ecs.RegisterComponent<Velocity>();
    ecs.RegisterComponent<Rotation>();
    ecs.RegisterComponent<Texture>();
    ecs.RegisterComponent<Thrust>();
    ecs.RegisterComponent<RotationVelocity>();
    ecs.RegisterComponent<BoxCollider>();
    ecs.RegisterComponent<CollideWithPlayer>();
    ecs.RegisterComponent<Follow>();
    ecs.RegisterComponent<DragCoefficient>();
    ecs.RegisterComponent<Speed>();
    ecs.RegisterComponent<Shooter>();

    // NOTE: Maybe add a component_id function which takes template for type to avoid always using typeid.
    System render_system{};
    render_system.name = "Render";
    render_system.requirements = ecs.ComponentBits<Texture>() | ecs.ComponentBits<Position>() | ecs.ComponentBits<Rotation>();
    render_system.Action = RenderAction;

    System move_system{};
    move_system.name = "Move";
    move_system.requirements = ecs.ComponentBits<Position>() | ecs.ComponentBits<Velocity>();
    move_system.Action = MoveAction;

    System rotate_system{};
    rotate_system.name = "Rotate";
    rotate_system.requirements = ecs.ComponentBits<Rotation>() | ecs.ComponentBits<RotationVelocity>();
    rotate_system.Action = RotateAction;

    System engine_system{};
    engine_system.name = "Engine";
    engine_system.requirements = ecs.ComponentBits<Thrust>() | ecs.ComponentBits<Velocity>() | ecs.ComponentBits<Rotation>();
    engine_system.Action = EngineAction;

    System inactive_on_leave_screen_system{};
    inactive_on_leave_screen_system.name = "InactiveOnLeaveScreen";
    inactive_on_leave_screen_system.requirements = ecs.ComponentBits<Position>();
    inactive_on_leave_screen_system.Action = SetInactiveOnLeaveScreenAction;

    System collision_system{};
    collision_system.name = "Collisions";
    collision_system.requirements = ecs.ComponentBits<BoxCollider>() | ecs.ComponentBits<Position>() | ecs.ComponentBits<CollideWithPlayer>();
    collision_system.Action = CollisionDetectAction;

    System follow_system{};
    follow_system.name = "Follow";
    follow_system.requirements = ecs.ComponentBits<Follow>() | ecs.ComponentBits<Position>();
    follow_system.Action = FollowAction;

    System drag_system{};
    drag_system.name = "Drag";
    drag_system.requirements = ecs.ComponentBits<DragCoefficient>() | ecs.ComponentBits<Velocity>();
    drag_system.Action = DragAction;

    System shooter_system{};
    shooter_system.name = "Shooter";
    shooter_system.requirements = ecs.ComponentBits<Shooter>() | ecs.ComponentBits<Position>() | ecs.ComponentBits<Texture>() | ecs.ComponentBits<Rotation>();
    shooter_system.Action = ShooterAction;
    
    // Per frame systems
    ecs.RegisterSystem(shooter_system);
    ecs.RegisterSystem(inactive_on_leave_screen_system); // Checking and disabling textures in more expensive than just letting them render.
    ecs.RegisterSystem(collision_system);
    ecs.RegisterSystem(rotate_system);
    ecs.RegisterSystem(engine_system);
    ecs.RegisterSystem(drag_system);
    ecs.RegisterSystem(move_system);
    ecs.RegisterSystem(follow_system);
    ecs.RegisterSystem(render_system); // Render Last -> Might even want to take out of loop and call myself.
}

void AsteroidsGame::ManageEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // This is kind of hacky but saves having to do many checks for game state.
        static State booster_was_on{};
        switch (event.type) {
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_KEYDOWN:
                if (event.key.repeat != 0) continue;
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_W:
                        ecs.GetComponent<Thrust>(player).value = 1.0f;
                        if (update_deltatime) {
                            ecs.entity_state[booster] = ACTIVE;
                        } else {
                            ecs.entity_state[booster] = booster_was_on;
                        }
                        break;
                    case SDL_SCANCODE_A:
                        ecs.GetComponent<RotationVelocity>(player).value -= 10.0f;
                        break;
                    case SDL_SCANCODE_D:
                        ecs.GetComponent<RotationVelocity>(player).value += 10.0f;
                        break;
                    case SDL_SCANCODE_SPACE:
                        // SHOOT;
                        ecs.GetComponent<Shooter>(player).shooting = true;
                        break;
                    case SDL_SCANCODE_ESCAPE:
                        deltatime = 0.0;
                        update_deltatime = false;
                        booster_was_on = ecs.entity_state[booster];
                        break;
                    default:
                        break;
                }
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_W:
                        ecs.GetComponent<Thrust>(player).value = 0.0f;
                        
                        if (update_deltatime) {
                            ecs.entity_state[booster] = SLEEP;
                        } else {
                            ecs.entity_state[booster] = booster_was_on;
                        }
                        break;
                    case SDL_SCANCODE_A:
                        ecs.GetComponent<RotationVelocity>(player).value += 10.0f;
                        break;
                    case SDL_SCANCODE_D:
                        ecs.GetComponent<RotationVelocity>(player).value -= 10.0f;
                        break;
                    case SDL_SCANCODE_SPACE:
                        // STOP SHOOT;
                        ecs.GetComponent<Shooter>(player).shooting = false;
                        break;
                    case SDL_SCANCODE_ESCAPE:
                        update_deltatime = true;
                        ecs.entity_state[booster] = ecs.GetComponent<Thrust>(player).value > 0.0f ? ACTIVE : SLEEP;
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


TextureID AsteroidsGame::GetTexture(const std::string& texture_path) {
    auto texture = texture_id_map.find(texture_path);
    if (texture != texture_id_map.end()) return texture_id_map[texture_path];

    // Otherwise doesnt exist and we need to init.
    static unsigned int texture_id = 0;
    SDL_Texture* sdl_texture = IMG_LoadTexture(renderer, texture_path.c_str());
    texture_id_map[texture_path] = texture_id;
    texture_map[texture_id_map[texture_path]] = sdl_texture;

    texture_id++;

    return texture_id_map[texture_path];
}