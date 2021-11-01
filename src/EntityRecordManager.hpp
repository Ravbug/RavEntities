#pragma once
#include <vector>
#include <queue>
#include <array>
#include <type_traits>
#include "World.hpp"
#include "Types.hpp"
#include "Component.hpp"
#include <variant>

// a variadic contains helper
template<typename T, typename... Ts>
static constexpr bool contains(){
    return std::disjunction_v<std::is_same<T, Ts>...>;
}

template <typename T, typename... Ts>
struct Index;

template <typename T, typename... Ts>
struct Index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct Index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + Index<T, Ts...>::value> {};

template <typename T, typename... Ts>
constexpr std::size_t Index_v = Index<T, Ts...>::value;

struct EntityRecordManager{
    EntityRecordManager() = delete;
    
    static World dummyWorld;
    
    template<typename ... A>
    struct EntityRecord{
        inline static constexpr std::size_t ntypes = sizeof ... (A);
        std::array<pos_t,ntypes> routingTable;
        std::reference_wrapper<World> world;
        
        static inline bool IsValid(pos_t pos){
            return pos != INVALID_INDEX;
        }
        
        inline void Reset(){
            for(const auto pos : routingTable){
                if (!IsValid(pos)){
                    //TODO: for all that are not INVALID, we must destroy those components
                }
            }
            
            std::fill(routingTable.begin(), routingTable.end(), INVALID_INDEX);
        }
        
        EntityRecord() : world(dummyWorld){
            // signifies that the entity for this record has no components
            Reset();
        }
        
        template<typename Comp_T>
        constexpr inline pos_t& GetLocation(){
            static_assert(contains<Comp_T, A...>(), "Passed type is not valid for this entity!");
            
            // calculate the position index for this type
            auto type_idx = Index_v<Comp_T, A...>;
            
            //returns a reference so it can be modified
            return routingTable[type_idx];
        }
    };
    
    // generating multiple versions of these members
    template<typename ... A>
    static std::vector<EntityRecord<A...>> recordData;
    
    template<typename ... A>
    static std::queue<entity_id_t> vendList;
    
    // invoked by the entity constructor
    template<typename ... A>
    static inline entity_id_t CreateEntity(){

        // is there an item in the vend list? if so, this is our ID
        entity_id_t id;
        if (!vendList<A...>.empty()){
            id = vendList<A...>.front();
            vendList<A...>.pop();
        }
        else{
            // otherwise we have a new ID which is the next location we use in the vector
            id = recordData<A...>.size();
        }
        
        // add the record data struct
        recordData<A...>.emplace_back();
        
        return id;
    }
    
    // invoked by entity->destroy
    template<typename ... A>
    static inline void DestroyEntity(entity_id_t id){
        // return its ID to the vend list
        assert(id < vendList<A...>.size());
        
        // wipe the record data
        recordData<A...>.Reset();
        
        // mark the ID as reusable
        vendList<A...>.push(id);
    }
    
    template<typename Entity_t, typename Comp_T, typename ... A>
    static inline ComponentHandle<Comp_T> GetComponent(Entity_t id){
        auto& record = recordData<A...>[id.id];
        auto& idx = record.template GetLocation<Comp_T>();
        return ComponentHandle<Comp_T>(record.world,idx);
    }
    
    template<typename Entity_t, typename Comp_T, typename ... A, typename ... Ctor_Args>
    static ComponentHandle<Comp_T> EmplaceComponent(Entity_t id,Ctor_Args ... args){
        // create the component in the World, then return the ref handle to it
        auto& record = recordData<A...>[id.id];
        auto ref = record.world.get().template EmplaceComponent<Entity_t,Comp_T>(id, args...);
        // we know where it was created based on the return value, so we now update the record's appropriate slot
        auto& idx = record.template GetLocation<Comp_T>();
        idx = ref.sparseindex;
        return ref;
    }
    
    template<typename Entity_t, typename Comp_T,  typename ... A>
    static inline void DestroyComponent(Entity_t id){
        // create the component reference
        auto& record = recordData<A...>[id.id];
        auto& idx = record.template GetLocation<Comp_T>();
        
        ComponentHandle<Comp_T> ref(record.world,idx);
        
        record.world.get().template DestroyComponent(ref);
        
        idx = INVALID_INDEX;
    }
    
    template<typename Entity_t, typename ... A>
    inline static std::optional<std::reference_wrapper<World>> GetEntityWorld(Entity_t id){
        auto& record = recordData<A...>[id.id];
        std::optional<std::reference_wrapper<World>> world;
        if (&record.world.get() != &dummyWorld){
            world.emplace(record.world);
        }
        return world;
    }
};

// static member definitions
template<typename ... A>
std::vector<EntityRecordManager::EntityRecord<A...>> EntityRecordManager::recordData;

template<typename ... A>
std::queue<entity_id_t> EntityRecordManager::vendList;
