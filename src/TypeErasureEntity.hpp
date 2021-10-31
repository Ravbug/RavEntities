#pragma once
#include <any>

struct TypeErasureEntity{
    std::any owner;
    
    template<typename T>
    TypeErasureEntity(const T& entity){
        // create the erasure wrappers here
        owner = entity;
    }
    
    template<typename T>
    T& As() const{
        //TODO: implement
    };
    
};

