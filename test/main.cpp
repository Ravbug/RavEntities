#include <EntityRecordManager.hpp>
#include <Entity.hpp>
#include <typeindex>
#include <iostream>
#include "CTTI.hpp"

using namespace std;

struct IntComponent : public RavEngine::AutoCTTI{
    int value;
};

struct FloatComponent : public RavEngine::AutoCTTI{
    float value;
};

struct TestEntity : public Entity<IntComponent,FloatComponent>{
    
};

int main(){
    TestEntity t;
    
    t.EmplaceComponent<IntComponent>(5);
    t.EmplaceComponent<FloatComponent>(7.4);

    t.DestroyComponent<IntComponent>();
    
    cout << type_index(typeid(t)).name() << endl;
}
