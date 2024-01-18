#pragma once

#include <array>
#include <cassert>
#include <iostream>
#include <unordered_map>

template<typename T>
struct ObjectPoolItem {
    T object;
    bool alive = false;
};

template <typename T, int C>
struct ObjectPool {
    std::array<ObjectPoolItem<T>, C> objects;

    std::unordered_map<T, int> object_to_id;

    int last_checked = 0;

    void InitMap() {
        for (int i = 0; i < C; i++) {
            object_to_id[objects[i].object] = i;
        }
    }

    ObjectPoolItem<T>& ObtainObject() {
        int to_check = last_checked + C;
        for (last_checked; last_checked < to_check; ++last_checked) {
            if (!objects[last_checked % C].alive) {
                auto& to_return = objects[last_checked % C];
                last_checked = (last_checked + 1) % C;
                return to_return;
            }
        }
        std::cout << "No free Objects, try allocating a bigger pool.";
        return objects[0];
        // assert(false);
    }
};