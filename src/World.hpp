#pragma once
#include "unordered_vector.hpp"
#include <queue>
#include "Types.hpp"
#include "TypeErasureEntity.hpp"
#include "Component.hpp"
#include "CTTI.hpp"
#include <unordered_map>
#include <any>

struct World;

template<typename T, typename vec = std::vector<T>>
class SparseComponentStore{
    struct aux_data{
        TypeErasureEntity owner;
        pos_t posInSparse;
        aux_data(const decltype(owner)& o, const decltype(posInSparse) p) : owner(o), posInSparse(p){}
    };
    
    unordered_vector<T,vec> dense_set;
    unordered_vector<aux_data,std::vector<aux_data>> aux_set;
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
        aux_set.emplace(entity,idx);
        
        return ComponentHandle<T>(*world,idx);
    }
    
    /**
     Remove a component on an entity
     */
    inline void DestroyComponent(const ComponentHandle<T>& handle){
        assert(&handle.world.get() == world);   // this handle is not valid for this world!
        assert(handle.sparseindex < sparse_set.size());
        
        // At this point, the EntityRecordManager has already been taken care of
        
        auto destroyIdx = sparse_set[handle.sparseindex];
        dense_set.erase(dense_set.begin() + destroyIdx);
        aux_set.erase(aux_set.begin() + destroyIdx);
        
        // this sparse index is now reusable
        vendlist.push(handle.sparseindex);
        sparse_set[handle.sparseindex] = INVALID_INDEX;
        
        // since this call could have moved memory, we need to update that
        if (!dense_set.empty()){
            // what is there now?
            auto& currentAux = aux_set[destroyIdx];
            sparse_set[currentAux.posInSparse] = destroyIdx;    // point it at the location we just filled
        }
    }
    
};

struct World{
    
    std::unordered_map<RavEngine::ctti_t, std::any> component_map;
    
    template<typename T>
    inline void MakeIfNotExists(){
        constexpr auto id = RavEngine::CTTI<T>();
        auto it = component_map.find(id);
        if (it == component_map.end()){
            component_map.insert(std::make_pair(id,SparseComponentStore<T>(this)));
        }
    }
    
    template<typename Entity_t, typename T, typename ... Args>
    inline ComponentHandle<T> EmplaceComponent(Entity_t entity, Args ... arguments){
        MakeIfNotExists<T>();
        auto row = std::any_cast<SparseComponentStore<T>>(&component_map[RavEngine::CTTI<T>()]);
        return row->template EmplaceComponent(entity,arguments...);
    }
    
    template<typename T>
    inline void DestroyComponent(const ComponentHandle<T>& handle){
        auto row = std::any_cast<SparseComponentStore<T>>(&component_map[RavEngine::CTTI<T>()]);
        row->DestroyComponent(handle);
    }
};
