#pragma once
#include "EntityRecordManager.hpp"
#include "Component.hpp"

struct World;

template<typename ... A>
struct Entity{
    
    friend class World;
    
    entity_id_t id = INVALID_INDEX;
    
    Entity(){}
    
    inline void Create(){
        id = EntityRecordManager::CreateEntity<A...>();
    }
    
    inline void Destroy(){
        EntityRecordManager::DestroyEntity<A...>(id);
    }
    
    template<typename Comp_T, typename ... Args>
    inline ComponentHandle<Comp_T> EmplaceComponent(Args ... ctor_args) const{
        assert(IsValid());
        auto& myself = *this;
        return EntityRecordManager::EmplaceComponent<decltype(myself), Comp_T, A...>(myself, ctor_args...);
    }
    
    template<typename Comp_T>
    inline void DestroyComponent() const{
        assert(IsValid());
        auto& myself = *this;
        EntityRecordManager::DestroyComponent<decltype(myself),Comp_T,A...>(myself);
    }
    
    template<typename Comp_T>
    inline ComponentHandle<Comp_T> GetComponent() const{
        assert(IsValid());
        auto& myself = *this;
        return EntityRecordManager::GetComponent<decltype(myself), Comp_T, A...>(myself);
    }
    
    inline std::optional<std::reference_wrapper<World>> GetWorld() const{
        assert(IsValid());
        auto& myself = *this;
        return EntityRecordManager::GetEntityWorld<decltype(myself),A...>(myself);
    }
    
    inline bool IsValid() const{
        return id != INVALID_INDEX;
        //TODO: check with entity record manager to ensure this ID is actually in use
    }
    
    inline operator bool() const{
        return IsValid();
    }
    
private:
    inline void MoveToWorld(World* newWorld) const{
        assert(IsValid());
        auto& myself = *this;
        EntityRecordManager::MoveToWorld<decltype(myself),A...>(myself,newWorld);
    }
    
    inline void ReturnToStaging() const{
        assert(IsValid());
        auto& myself = *this;
        EntityRecordManager::Despawn<decltype(myself), A...>(myself);
    }
};

