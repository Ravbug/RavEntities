#include "Registry.hpp"
#include "World.hpp"
#include "Entity.hpp"
#include <iostream>
#include <array>
#include <chrono>

using namespace std;

struct IntComponent : public RavEngine::AutoCTTI{
    int value;
};

struct FloatComponent : public RavEngine::AutoCTTI{
    float value;
};

struct MyPrototype : public Entity{
    void Create(){
        auto& comp = EmplaceComponent<IntComponent>();
        comp.value = 5;
    }
};

struct MyExtendedPrototype : public MyPrototype{
    void Create(){
        MyPrototype::Create();
        auto& comp = EmplaceComponent<FloatComponent>();
        comp.value = 7.5;
    }
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
    // perf tests
    {
        World w;
        constexpr auto n_entities = 20'000'000;
        auto dur = time([&]{
            for(int i = 0; i < n_entities; i++){
                w.CreatePrototype<MyExtendedPrototype>();
            }
        });
        cout << "Spawning " << n_entities << " with 2 components took " << dur.count() << "µs\n";
        
        dur = time([&]{
            w.Filter<IntComponent>([](auto& ic){
                ic.value *= 2;
            });
        });
        cout << "Single component filter on " << n_entities << " took " << dur.count() << "µs\n";
        
        dur = time([&]{
            w.Filter<IntComponent,FloatComponent>([](auto& ic, auto& fc){
                ic.value /= 3;
                fc.value = ic.value * 6;
            });
        });
        cout << "Two-component (worst) filter on " << n_entities << " entities took " << dur.count() << "µs\n";
        
        dur = time([&]{
            w.Filter<FloatComponent,IntComponent>([](auto& fc, auto& ic){
                ic.value /= 3;
                fc.value = ic.value * 6;
            });
        });
        cout << "Two-component (best) filter on " << n_entities << " entities took " << dur.count() << "µs\n";
    }
}
