#pragma once
#include <vector>
#include <memory>
#include <iterator>  // Para std::make_move_iterator

template <typename T>
std::vector<std::unique_ptr<T>> vector_concat(
    std::vector<std::unique_ptr<T>> vec1,
    std::vector<std::unique_ptr<T>> vec2
) {
    std::vector<std::unique_ptr<T>> result;
    
    // Mover elementos del primer vector al resultado
    result.reserve(vec1.size() + vec2.size());
    for (auto& ptr : vec1) {
        result.push_back(std::move(ptr));
    }

    // Mover elementos del segundo vector al resultado
    for (auto& ptr : vec2) {
        result.push_back(std::move(ptr));
    }

    return result;
}