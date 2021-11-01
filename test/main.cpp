#include <EntityRecordManager.hpp>
#include <Entity.hpp>
#include <typeindex>
#include <iostream>
#include "CTTI.hpp"

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

int main(){
    std::array<TestEntity, 10> entities;
    
    auto comp = entities[0].GetComponent<FloatComponent>();
    (*comp).value = 10;
    
    auto comp2 = entities[0].GetComponent<FloatComponent>();
    cout << (*comp).value << endl;
    
    TypeErasureEntity te(entities[0]);
    auto e = te.As<TestEntity>();
    assert(!e.GetWorld());
    
    auto owner = comp.GetOwner<Entity<IntComponent,FloatComponent>>();
    
    
    for (auto& e : entities){
        e.DestroyComponent<IntComponent>();
    }
    
    World w;
    TestEntity e11;
    (*e11.GetComponent<FloatComponent>()).value = 46;
    cout << (*e11.GetComponent<FloatComponent>()).value << endl;
    w.Spawn(e11);
    
    
    cout <<  (*e11.GetComponent<FloatComponent>()).value << endl;
    
    for(auto& comp : w.GetAllComponentsOfType<FloatComponent>()){
        comp.value += 5;
    }
    
    assert(e11.GetWorld());
    w.Despawn(e11);
    assert(!e11.GetWorld());

    //comp->value = 5;
}
