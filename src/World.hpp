#pragma once
#include "unordered_vector.hpp"
#include <queue>
#include "Types.hpp"
#include "TypeErasureEntity.hpp"
#include "Component.hpp"
#include "CTTI.hpp"
#include <unordered_map>

struct World;

template<typename T, typename vec = std::vector<T>>
class SparseComponentStore{
    struct aux_data{
        TypeErasureEntity owner;
        pos_t posInSparse;
    };
    
    unordered_vector<T,vec> dense_set;
    unordered_vector<aux_data,vec> aux_set;
    std::vector<pos_t> sparse_set;
    std::queue<pos_t> vendlist;
    
    World* world = nullptr;
public:
    /**
     Must tell the sparse map what the owning world is
     */
    SparseComponentStore(decltype(world) wptr ) : world(wptr){}
    
    inline auto Size() const{
        return dense_set.size();
    }
    
    /**
     Create a component on an entity
     */
    template<typename Entity_t, typename ... Args>
    inline ComponentHandle<T> EmplaceComponent(Entity_t entity, Args ... arguments){
        // add the component to the dense set
        dense_set.emplace(arguments...);
        
        
        // find the sparse location
        // if there's a hole, fill it, otherwise set to the end
        pos_t idx;
        if (!vendlist.empty()){
            idx = vendlist.front();
            vendlist.pop();
            sparse_set[idx] = dense_set.size() - 1;
        }
        else{
            idx = sparse_set.size();
            sparse_set.push_back(dense_set.size() - 1);
        }
        
        // add the component bookkeeping data
        aux_set.emplace({entity,idx});
        
        return ComponentHandle<T>(world,idx);
    }
    
    /**
     Remove a component on an entity
     */
    template<typename Entity_t>
    inline void DestroyComponent(const ComponentHandle<T>& handle){
        assert(handle.world != world);   // this handle is not valid for this world!
        assert(handle.sparseindex < dense_set.size());
        
        // At this point, the EntityRecordManager has already been taken care of
        
        auto destroyIdx = sparse_set[handle.sparseindex];
        dense_set.erase(dense_set.begin() + destroyIdx);
        aux_set.erase(aux_set.begin() + destroyIdx);
        
        // since this call could have moved memory, we need to update that
        if (!dense_set.empty()){
            // what is there now?
            auto& currentAux = aux_set[destroyIdx];
            sparse_set[currentAux.posInSparse] = destroyIdx;    // point it at the location we just filled
        }
    }
    
};

struct World{
    
    std::unordered_map<RavEngine::ctti_t, int> component_map;
    
    template<typename Entity_t, typename T, typename ... Args>
    inline ComponentHandle<T> EmplaceComponent(entity_id_t entity, Args ... arguments){
        auto& row = component_map[RavEngine::CTTI<T>()];
    }
    
    template<typename Entity_t, typename T>
    inline void DestroyComponent(const ComponentHandle<T>& handle){
        auto& row = component_map[RavEngine::CTTI<T>()];
    }
};
