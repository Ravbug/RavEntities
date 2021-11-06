#include "Registry.hpp"
#include "World.hpp"
#include "Entity.hpp"
#include <iostream>
#include <array>

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

int main(){
    World w;
    
    std::array<MyPrototype, 50> entities;
    for(auto& e : entities){
        e = w.CreatePrototype<MyPrototype>();
    }

    std::array<MyExtendedPrototype, 50> exEntities;
    for(auto& e : exEntities){
        e = w.CreatePrototype<MyExtendedPrototype>();
    }
    
    entities[20].Destroy();
    
    int n_loops = 0;
    w.Filter<IntComponent>([&](const IntComponent& ic){
        n_loops ++;
    });
    cout << n_loops << " have IntComponent" << endl;
    
    n_loops = 0;
    w.Filter<IntComponent,FloatComponent>([&](const auto& ic, const auto& fc){
        n_loops ++;
    });
    cout << n_loops << " have IntComponent and FloatComponent" << endl;
}
