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
    World w;
    auto dur = time([&]{
        for(int i = 0; i < n_entities; i++){
            TestEntity t;
            w.Spawn(t);
        }
    });
    cout << "Spawning " << n_entities << " entities took " << dur.count() << "µs\n";
   
    dur = time([&]{
        for(auto& intcomp : w.GetAllComponentsOfType<IntComponent>()){
            intcomp.value += 5;
        }
    });
    cout << "Updating " << n_entities << " components took " << dur.count() << "µs\n";
}
