#include "Registry.hpp"
#include "World.hpp"
#include "Entity.hpp"
#define STATIC(a) decltype(a) a

STATIC(Registry::available);
STATIC(Registry::entityData);

entity_t World::CreateEntity(){
    entity_t id;
    if (available.size() > 0){
        id = available.front();
        available.pop();
    }
    else{
        id = localToGlobal.size();
        localToGlobal.push_back(INVALID_ENTITY);
    }
    localToGlobal[id] = Registry::CreateEntity(this, id);
    return localToGlobal[id];
}
