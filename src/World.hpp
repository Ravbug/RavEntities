#pragma once
#include "unordered_vector.hpp"
#include <queue>
#include "CTTI.hpp"
#include <unordered_map>
#include <any>
#include <tuple>

struct Entity;

template <typename T, typename... Ts>
struct Index;

template <typename T, typename... Ts>
struct Index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct Index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + Index<T, Ts...>::value> {};

template <typename T, typename... Ts>
constexpr std::size_t Index_v = Index<T, Ts...>::value;

class World{
    
    std::vector<entity_t> localToGlobal;
    std::queue<entity_t> available;
    
    friend class Entity;
    friend class Registry;
    
    template<typename T>
    class SparseSet{
        unordered_vector<T> dense_set;
        unordered_vector<entity_t> aux_set;
        std::vector<entity_t> sparse_set;
        
    public:
        
        template<typename ... A>
        inline T& Emplace(entity_t local_id, A ... args){
            dense_set.emplace(args...);
            aux_set.emplace(local_id);
            sparse_set.resize(local_id+1,INVALID_ENTITY);  //ensure there is enough space for this id
            
            sparse_set[local_id] = dense_set.size()-1;
            return dense_set[dense_set.size()-1];
        }
        
        inline void Destroy(entity_t local_id){
            assert(local_id < sparse_set.size());
            // call the destructor
            dense_set.erase(dense_set.begin() + sparse_set[local_id]);
            aux_set.erase(aux_set.begin() + sparse_set[local_id]);
            
            // update the location it points
            auto owner = aux_set[sparse_set[local_id]];
            sparse_set[owner] = local_id;
            sparse_set[local_id] = INVALID_INDEX;
        }
        
        inline bool HasComponent(entity_t local_id){
            return sparse_set[local_id] != INVALID_ENTITY;
        }
        
        auto begin(){
            return dense_set.begin();
        }
        
        auto end(){
            return dense_set.end();
        }
        
        auto begin() const{
            return dense_set.begin();
        }
        
        auto end() const{
            return dense_set.end();
        }
        
        // get by dense index, not by entity ID
        T& Get(entity_t idx){
            return dense_set[idx];
        }
        
        auto GetOwner(entity_t idx){
            return aux_set[idx];
        }
        
        auto DenseSize() const{
            return dense_set.size();
        }
    };
    
    struct SparseSetErased{
        std::any set;
        std::function<void(entity_t id)> destroyFn;
        
        template<typename T>
        inline SparseSet<T>* GetSet() {
            return std::any_cast<SparseSet<T>>(&set);
        }
        
        template<typename T>
        SparseSetErased(const SparseSet<T>& s) : set(s), destroyFn([&](entity_t local_id){
            auto ptr = GetSet<T>();
            if (ptr->HasComponent(local_id)){
                ptr->Destroy(local_id);
            }
        }){}
    };
    
    std::unordered_map<RavEngine::ctti_t, SparseSetErased> componentMap;

    inline void Destroy(entity_t local_id){
        // go down the list of all component types registered in this world
        // and call destroy if the entity has that component type
        // possible optimization: vector of vector<ctti_t> to make this faster?
        for(const auto& pair : componentMap){
            pair.second.destroyFn(local_id);
        }
    }
    
    template<typename T>
    inline SparseSet<T>* MakeIfNotExists(){
        auto id = RavEngine::CTTI<T>();
        auto it = componentMap.find(id);
        if (it == componentMap.end()){
            it = componentMap.emplace(std::make_pair(id,SparseSet<T>())).first;
        }
        auto ptr = (*it).second.template GetSet<T>();
        assert(ptr != nullptr);
        return ptr;
    }
    
    template<typename T, typename ... A>
    inline T& EmplaceComponent(entity_t local_id, A ... args){
        auto ptr = MakeIfNotExists<T>();
        
        //TODO: detect if T constructor's first argument is an entity_t, if it is, then we need to pass that before args (pass local_id again)
        return ptr->Emplace(local_id,args...);
    }
    
    template<typename T>
    inline void DestroyComponent(entity_t local_id){
        // set owner of that slot to invalid_entity
        // manually invoke the destructor on the T at that slot
    }
    
    template<typename T>
    inline SparseSet<T>* GetRange(){
        auto& set = componentMap.at(RavEngine::CTTI<T>());
        return set.template GetSet<T>();
    }
    
    template<typename T>
    inline void FilterValidityCheck(entity_t id, SparseSetErased* set, bool& satisfies){
        // in this order so that the first one the entity does not have aborts the rest of them
        satisfies = satisfies && set->template GetSet<T>()->HasComponent(id);
    }
    
    template<typename T>
    inline T& FilterComponentGet(entity_t idx, SparseSetErased* ptr){
        return ptr->template GetSet<T>()->Get(idx);
    }
   
    template<typename T>
    inline SparseSetErased* FilterGetSparseSet(){
        return &(componentMap.at(RavEngine::CTTI<T>()));
    }
    
public:
    entity_t CreateEntity();
    
    template<typename T, typename ... A>
    inline T CreatePrototype(A ... args){
        auto id = CreateEntity();
        T en;
        en.id = id;
        en.Create(args...);
        return en;
    }
    
    template<typename ... A, typename func>
    inline void Filter(const func& f){
        constexpr auto n_types = sizeof ... (A);
        static_assert(n_types > 0, "Must supply a type to query for");
        
        auto mainFilter = GetRange<typename std::tuple_element<0, std::tuple<A...> >::type>();
        
        if constexpr (n_types == 1){
            for(size_t i = 0; i < mainFilter->DenseSize(); i++){
                auto& item = mainFilter->Get(i);
                f(item);
            }
        }
        else{
            std::array<SparseSetErased*, n_types> ptrs{ FilterGetSparseSet<A>()...};
            // does this entity have all of the other required components?
            for(size_t i = 0; i < mainFilter->DenseSize(); i++){
                const auto owner = mainFilter->GetOwner(i);
                if (EntityIsValid(owner)){
                    bool satisfies = true;
                    (FilterValidityCheck<A>(owner, ptrs[Index_v<A, A...>], satisfies), ...);
                    if (satisfies){
                        f(FilterComponentGet<A>(i,ptrs[Index_v<A, A...>])...);
                    }
                }
            }
        }
    }
    
    ~World(){
        //TODO: destroy all entities 
    }
};
