#include <EntityRecordManager.hpp>
#include <Entity.hpp>

#include <typeindex>
#include <iostream>

using namespace std;

struct IntComponent{
    int value;
};

struct FloatComponent{
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
