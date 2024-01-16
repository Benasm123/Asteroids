#include <array>
#include <cassert>

template<typename T>
struct ObjectPoolItem {
    T object;
    bool alive = false;
};

template <typename T, int C>
struct ObjectPool {
    std::array<ObjectPoolItem<T>, C> objects;
    const int size = C;

    ObjectPoolItem<T>& ObtainObject() {
        for (int i = 0; i < size; ++i) {
            if (!objects[i].alive) return objects[i];
        }
        std::cout << "No free Objects, try allocating a bigger pool.";
        assert(false);
    }
};