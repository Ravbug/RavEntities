#include <EntityRecordManager.hpp>
#include <Entity.hpp>
#include <typeindex>
#include <iostream>
#include "CTTI.hpp"
#include <chrono>

using namespace std;

struct IntComponent : public RavEngine::AutoCTTI{
    int value;
    IntComponent(decltype(value) v ): value(v){}
};

struct FloatComponent : public RavEngine::AutoCTTI{
    float value;
    FloatComponent(decltype(value) v ) : value(v){}
};

struct TestEntity : public Entity<IntComponent,FloatComponent>{
    TestEntity(){
        auto comp = EmplaceComponent<IntComponent>(5);
        comp.GetOwner<Entity<IntComponent,FloatComponent>>();
        EmplaceComponent<FloatComponent>(7.4f);
    }
};

struct test{
    int x;
    float y;
};

struct test2{
    std::string fatthing;
};

static std::chrono::system_clock timer;

template<typename T>
static inline decltype(timer)::duration time(const T& func){
    auto begin_time = timer.now();
    func();
    auto end_time = timer.now();
    return chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time);
}


int main(){
    constexpr auto n_entities = 3'000'000;
    std::unique_ptr<std::array<TestEntity,n_entities>> entities;
    World w;
    
    auto dur = time([&]{
        entities = std::make_unique<std::array<TestEntity,n_entities>>();
    });
    cout << "Creating " << n_entities << " entities took " << dur.count() << "µs\n";

    dur = time([&]{
        for(const auto& e : *entities){
            w.Spawn(e);
        }
    });
    cout << "Spawning " << n_entities << " entities took " << dur.count() << "µs\n";
   
    dur = time([&]{
        for(auto& intcomp : w.GetAllComponentsOfType<IntComponent>()){
            intcomp.value += 5;
        }
    });
    
    cout << "Updating " << n_entities << " components took " << dur.count() << "µs\n";
    
    dur = time([&]{
        for(const auto& e : *entities){
            (*e.GetComponent<IntComponent>()).value += 6;
        }
    });
    
    cout << "GetComponent on " << n_entities << " entities took " << dur.count() << "µs\n";
    
    cout << "Before destroying, GetComponent returns " << ((*entities)[0].GetComponent<IntComponent>().IsValid() ? "valid" : "invalid") << "\n";
    
    dur = time([&]{
        for(const auto& e : *entities){
            e.DestroyComponent<IntComponent>();
        }
    });
    
    cout << "After destroying, GetComponent returns " << ((*entities)[0].GetComponent<IntComponent>().IsValid() ? "valid" : "invalid") << "\n";
    
    cout << "Destroy 1 component on " << n_entities << " entities took " << dur.count() << "µs\n";
    
    dur = time([&]{
        for(const auto& e : *entities){
            e.EmplaceComponent<IntComponent>(67);
        }
    });
    cout << "Emplace 1 component pre-allocated on " << n_entities << " entities took " << dur.count() << "µs\n";
}
