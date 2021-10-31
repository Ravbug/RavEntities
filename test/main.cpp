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
        EmplaceComponent<IntComponent>(5);
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
    for(auto& e : entities){
        e = TestEntity();
    }
    
    for (auto& e : entities){
        e.DestroyComponent<IntComponent>();
    }
}
