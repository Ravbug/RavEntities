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
    
};

struct test{
    int x;
    float y;
};

struct test2{
    std::string fatthing;
};

int main(){
    TestEntity t;
    
    t.EmplaceComponent<IntComponent>(5);
    t.EmplaceComponent<FloatComponent>(7.4f);

    t.DestroyComponent<IntComponent>();
    
    cout << type_index(typeid(t)).name() << endl;
}
