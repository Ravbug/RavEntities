#include "Registry.hpp"
#include "World.hpp"
#include "Entity.hpp"
#include "ComponentHandle.hpp"
#include <iostream>
#include <array>
#include <chrono>

using namespace std;

struct IntComponent {
    int value;
};

struct FloatComponent{
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


template<typename T>
static auto time(const T& func){
    auto begin_time = std::chrono::system_clock::now();
    func();
    auto end_time = std::chrono::system_clock::now();
    return chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time);
}

int main() {
    // perf tests
    {
        World w;
        constexpr auto n_entities =
#ifdef _DEBUG
            2'000;
#else
            20'000'000;
#endif
        auto dur = time([&] {
            for (int i = 0; i < n_entities; i++) {
                w.CreatePrototype<MyExtendedPrototype>();
            }
            });
        cout << "Spawning " << n_entities << " with 2 components took " << dur.count() << "µs\n";

        dur = time([&] {
            w.Filter<IntComponent>([](auto& ic) {
                ic.value *= 2;
                });
            });
        cout << "Single component filter on " << n_entities << " took " << dur.count() << "µs\n";

        dur = time([&] {
            w.Filter<IntComponent, FloatComponent>([](auto& ic, auto& fc) {
                ic.value /= 3;
                fc.value = ic.value * 6;
                });
            });
        cout << "Two-component (worst) filter on " << n_entities << " entities took " << dur.count() << "µs\n";

        dur = time([&] {
            w.Filter<FloatComponent, IntComponent>([](auto& fc, auto& ic) {
                ic.value /= 3;
                fc.value = ic.value * 6;
                });
            });
        cout << "Two-component (best) filter on " << n_entities << " entities took " << dur.count() << "µs\n";
    }
    // filter tests
    {
        World w;
        auto e = w.CreatePrototype<Entity>();
        auto& ic = e.EmplaceComponent<IntComponent>();
        ic.value = 6;

        auto e2 = w.CreatePrototype<Entity>();
        e2.EmplaceComponent<FloatComponent>().value = 54.2;

        int count = 0;
        w.Filter<IntComponent, FloatComponent>([&](auto& ic, auto& fc) {
            count++;
            });
        assert(count == 0);
        cout << "A 2-filter with 0 possibilities found " << count << " results\n";

        w.Filter<IntComponent>([&](auto& ic) {
            ic.value *= 2;
        });
        
        ComponentHandle<IntComponent> handle(e);
        
        assert(handle->value == 6 * 2);

        e.DestroyComponent<IntComponent>();
        assert(e.HasComponent<IntComponent>() == false);
        count = 0;
        w.Filter<FloatComponent>([&](auto& fc) {
            count++;
        });
        cout << "After deleting the only intcomponent, the floatcomponent count is " << count << "\n";

        count = 0;
        w.Filter<IntComponent>([&](auto& fc) {
            count++;
        });
        cout << "After deleting the only intcomponent, the intcomponent count is " << count << "\n";

        assert((e.GetWorld() == e2.GetWorld()));
    }
    // create and destroy
    {
        World w;
        std::array<MyExtendedPrototype, 30> entities;
        for( auto& e : entities){
            e = w.CreatePrototype<MyExtendedPrototype>();
        }
        {
            int icount = 0;
            w.Filter<IntComponent>([&](auto& fc) {
                icount++;
            });
            int fcount = 0;
            w.Filter<FloatComponent>([&](auto& fc) {
                fcount++;
            });
            cout << "Spawning " << entities.size() << " 2-component entities yields " << icount << " intcomponents and " << fcount << " floatcomponents\n";
        }
        
        for(int i = 4; i < 20; i++){
            entities[i].Destroy();
        }
        
        {
            int icount = 0;
            w.Filter<IntComponent>([&](auto& fc) {
                icount++;
            });
            int fcount = 0;
            w.Filter<FloatComponent>([&](auto& fc) {
                fcount++;
            });
            cout << "After destroying " << 20-4 << " 2-component entities, filter yields " << icount << " intcomponents and " << fcount << " floatcomponents\n";
        }
    }
    // move between worlds
    {
        World w1, w2;
        
        std::array<MyPrototype, 10> w1entities;
        std::array<MyPrototype, 20> w2entities;
        
        for(auto& e : w1entities){
            e = w1.CreatePrototype<MyPrototype>();
        }
        
        for(auto& e : w2entities){
            e = w2.CreatePrototype<MyPrototype>();
        }
        
        int w1count = 0;
        w1.Filter<IntComponent>([&](auto& ic){
            ic.value = 1;
            w1count++;
        });
        
        int w2count = 0;
        w2.Filter<IntComponent>([&](auto& ic){
            ic.value = 2;
            w2count++;
        });
        
        cout << "w1count = " << w1count << ", w2count = " << w2count << "\n";
        
        // move some entities from w2 to w1
        for(int i = 0; i < w2entities.size()/2; i++){
            w2entities[i].MoveTo(w1);
        }
        
        w1count = 0;
        w1.Filter<IntComponent>([&](const auto& ic){
            w1count++;
            cout << ic.value << " ";
        });
        cout << "\n";
        w2count = 0;
        w2.Filter<IntComponent>([&](const auto& ic){
            w2count++;
            cout << ic.value << " ";
        });
        cout << "\nAfter moving entities to w1, w1count = " << w1count << ", w2count = " << w2count << "\n";
    }
}
   
