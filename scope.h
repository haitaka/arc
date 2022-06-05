#pragma once

#include <string>
#include <mutex>
#include <unordered_map>

template<typename T>
class Scope {
public:
    Scope() = default;
    T & getOrCreate(std::string const & name);
    void erase(std::string const & name);
private:
    std::mutex mutex;
    std::unordered_map<std::string, T> map;
};

template<typename T>
T & Scope<T>::getOrCreate(std::string const & name) {
    auto iterAndFlag = map.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                                   std::forward_as_tuple()); // FIXME ?
    auto iter = iterAndFlag.first;
    return iter->second;
}

template<typename T>
void Scope<T>::erase(std::string const & name) {
    map.erase(name);
}
