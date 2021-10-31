#pragma once
#include "EntityRecordManager.hpp"
#include "Component.hpp"

template<typename ... A>
struct Entity{
    entity_id_t id;
    
    Entity() : id(EntityRecordManager::CreateEntity<A...>()){}
    
    inline void Destroy(){
        EntityRecordManager::DestroyEntity<A...>(id);
    }
    
    template<typename Comp_T, typename ... Args>
    inline void EmplaceComponent(Args ... ctor_args){
        // pass the current class Type
        auto myself = *this;
        EntityRecordManager::EmplaceComponent<decltype(myself), Comp_T, A...>(myself, ctor_args...);
    }
    
    template<typename Comp_T>
    inline void DestroyComponent(){
        auto myself = *this;
        EntityRecordManager::DestroyComponent<decltype(myself),Comp_T,A...>(myself);
    }
    
    template<typename Comp_T>
    inline ComponentHandle<Comp_T> GetComponent() const{
        auto myself = *this;
        return EntityRecordManager::GetComponent<decltype(myself), Comp_T, A...>(myself);
    }
};

