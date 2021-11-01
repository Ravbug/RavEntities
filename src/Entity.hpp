#pragma once
#include "EntityRecordManager.hpp"
#include "Component.hpp"

struct World;

template<typename ... A>
struct Entity{
    entity_id_t id;
    
    Entity() : id(EntityRecordManager::CreateEntity<A...>()){}
    
    inline void Destroy(){
        EntityRecordManager::DestroyEntity<A...>(id);
    }
    
    template<typename Comp_T, typename ... Args>
    inline ComponentHandle<Comp_T> EmplaceComponent(Args ... ctor_args) const{
        auto myself = *this;
        return EntityRecordManager::EmplaceComponent<decltype(myself), Comp_T, A...>(myself, ctor_args...);
    }
    
    template<typename Comp_T>
    inline void DestroyComponent() const{
        auto myself = *this;
        EntityRecordManager::DestroyComponent<decltype(myself),Comp_T,A...>(myself);
    }
    
    template<typename Comp_T>
    inline ComponentHandle<Comp_T> GetComponent() const{
        auto myself = *this;
        return EntityRecordManager::GetComponent<decltype(myself), Comp_T, A...>(myself);
    }
    
    inline std::optional<std::reference_wrapper<World>> GetWorld() const{
        auto myself = *this;
        return EntityRecordManager::GetEntityWorld<decltype(myself),A...>(myself);
    }
    
    inline void MoveToWorld(World* newWorld) const{
        auto myself = *this;
        EntityRecordManager::MoveToWorld<decltype(myself),A...>(myself,newWorld);
    }
};

