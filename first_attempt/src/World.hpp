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
        aux_data(const aux_data& other) : owner(other.owner),posInSparse(other.posInSparse){}
    };
    
    unordered_vector<T,vec> dense_set;
    unordered_vector<aux_data,std::vector<aux_data>> aux_set;
    std::vector<pos_t> sparse_set;
    std::queue<pos_t> vendlist;
    
    std::reference_wrapper<World> world;
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
        
        return ComponentHandle<T>(world,idx);
    }
    
    /**
     Remove a component on an entity
     */
    inline void DestroyComponent(const ComponentHandle<T>& handle){
        assert(&handle.world.get() == &world.get());   // this handle is not valid for this world!
        assert(handle.sparseindex < sparse_set.size());
        
        // At this point, the EntityRecordManager has already been taken care of
        
        auto destroyIdx = sparse_set[handle.sparseindex];
        dense_set.erase(dense_set.begin() + destroyIdx);
        aux_set.erase(aux_set.begin() + destroyIdx);
        
        // this sparse index is now reusable
        vendlist.push(handle.sparseindex);
        sparse_set[handle.sparseindex] = INVALID_INDEX;
        
        // since this call could have moved memory, we need to update that
        if (destroyIdx < dense_set.size()){
            // what is there now?
            auto& currentAux = aux_set[destroyIdx];
            sparse_set[currentAux.posInSparse] = destroyIdx;    // point it at the location we just filled
        }
    }
    
    inline T* GetComponent(pos_t sparseidx){
        assert(sparseidx < sparse_set.size());
        assert(sparse_set[sparseidx] < dense_set.size());
        return &dense_set[sparse_set[sparseidx]];
    }
    
    inline bool IndexIsValid(pos_t sparseidx){
        return sparseidx < sparse_set.size() && sparse_set[sparseidx] != INVALID_INDEX;
    }
    
    template<typename Entity_t>
    inline Entity_t GetComponentOwner(pos_t sparseidx) const{
        auto& aux_data = aux_set[sparse_set[sparseidx]];
        auto eptr = aux_data.owner.template As<Entity_t>();
        return eptr;
    }
    
    inline bool IsValid(pos_t sparseidx){
        return sparseidx < sparse_set.size() && sparse_set[sparseidx] < dense_set.size() && sparse_set[sparseidx] != INVALID_INDEX;
    }
    
    inline auto begin(){
       return dense_set.begin();
    }
    
    inline auto begin() const{
        return dense_set.begin();
    }
    
    inline auto end(){
        return dense_set.end();
    }
    
    inline auto end() const{
        return dense_set.end();
    }
};

class World{
    friend class EntityRecordManager;
    template<typename T>
    friend class ComponentHandle;
private:
    std::unordered_map<RavEngine::ctti_t, std::any> component_map;
    
    template<typename T>
    inline SparseComponentStore<T>* MakeIfNotExists(){
        constexpr auto id = RavEngine::CTTI<T>();
        auto it = component_map.find(id);
        if (it == component_map.end()){
            it = component_map.emplace(std::make_pair(id,SparseComponentStore<T>(*this))).first;
        }
        auto ptr = std::any_cast<SparseComponentStore<T>>(&(*it).second);
        assert(ptr != nullptr);
        return ptr;
    }
    
    template<typename Entity_t, typename T, typename ... Args>
    inline ComponentHandle<T> EmplaceComponent(Entity_t entity, Args ... arguments){
        return MakeIfNotExists<T>()->template EmplaceComponent(entity,arguments...);
    }
    
    template<typename T>
    inline void DestroyComponent(const ComponentHandle<T>& handle){
        auto ptr = GetTypeRow<T>();
        if (ptr){
            ptr->DestroyComponent(handle);
        }
        else{
            assert(false);  // cannot destroy an invalid handle!
        }
    }
    
    template<typename T>
    inline T* GetComponent(pos_t sparseidx){
        auto ptr = GetTypeRow<T>();
        if (ptr){
            return ptr->GetComponent(sparseidx);
        }
        return nullptr;
    }
    
    template<typename Entity_t, typename T>
    inline Entity_t GetComponentOwner(pos_t sparseidx){
        auto ptr = GetTypeRow<T>();
        if (ptr){
            return ptr->template GetComponentOwner<Entity_t>(sparseidx);
        }
        else{
            return Entity_t();  // default to invalid handle
        }
    }
    
    template<typename T>
    inline SparseComponentStore<T>* GetTypeRow(){
        auto it = component_map.find(RavEngine::CTTI<T>());
        if (it == component_map.end()){
            return nullptr;
        }
        auto ptr = std::any_cast<SparseComponentStore<T>>(&(*it).second);
        assert(ptr != nullptr);
        return ptr;
    }
    
    template<typename T>
    inline bool ComponentIsValid(pos_t sparseidx){
        auto ptr = GetTypeRow<T>();
        if (ptr == nullptr){
            return false;
        }
        return ptr->IndexIsValid(sparseidx);
    }
    
public:
    template<typename Entity_t>
    inline void Spawn(Entity_t e){
        e.MoveToWorld(this);
    }
    
    template<typename Entity_t>
    inline void Despawn(Entity_t e){
        e.ReturnToStaging();
    }
    
    template<typename T>
    inline SparseComponentStore<T>& GetAllComponentsOfType(){
        return *GetTypeRow<T>();
    }
};
