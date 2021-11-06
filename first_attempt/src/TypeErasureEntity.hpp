#pragma once
#include <any>

struct TypeErasureEntity{
    std::any owner;
    
    template<typename T>
    TypeErasureEntity(const T& entity) : owner(entity){}
    
    /**
     Get the owner as a specific type. We assume the caller is correct.
     */
    template<typename T>
    const T As() const{
        return std::any_cast<T>(owner);
    };
    
};

