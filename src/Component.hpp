#pragma once
#include "TypeErasureEntity.hpp"
#include "Types.hpp"

struct Component{
    
};

struct World;

template<typename T>
struct ComponentHandle{
    std::reference_wrapper<World> world;
    pos_t sparseindex;
    
    ComponentHandle(const decltype(world)& w, decltype(sparseindex) idx) : world(w), sparseindex(idx){}

    // overload operator->
    T& operator*() const{
        return world.get().template GetComponent<T>(sparseindex);
    }
};

