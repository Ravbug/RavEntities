#pragma once

struct TypeErasureEntity{
    
    template<typename T>
    TypeErasureEntity(const T& entity){
        // create the erasure wrappers here
        //TODO: implement
    }
    
    template<typename T>
    T& As() const{
        //TODO: implement
    };
    
};

