#include <cassert>
#include <iostream>
#include "reference.h"

template<typename T>
Collectible<T>::Collectible(T && content) : content(content) {}

template<typename T>
StrongRef<T>::StrongRef() : referent(nullptr) {} // TODO used?

template<typename T>
StrongRef<T>::StrongRef(Collectible<T> * referent)
        : referent(referent) {
    referent->refCounter += 1;
    std::clog << "New strong reference to " << *referent << " @" << referent
              << " // counter is now " << referent->refCounter << std::endl;
}

template<typename T>
StrongRef<T>::StrongRef(StrongRef<T> && that) {
    swap(that);
}

template<typename T>
StrongRef<T>::~StrongRef() {
    if (referent != nullptr) {
        uint counter = referent->refCounter;
        counter -= 1;
        std::clog << "One less strong reference to " << *referent << " @" << referent;
        std::clog << " // counter is now " << counter;
        std::clog << std::endl;
        { // TODO synchronize
            referent->refCounter -= 1;
            if (referent->refCounter == 0) {
                delete referent;
                // TODO clean other weak references
            }
        }
    }
}

template<typename T>
StrongRef<T> & StrongRef<T>::operator=(StrongRef<T> && that) {
    swap(that);
    return *this;
}

template<typename T>
void StrongRef<T>::swap(StrongRef<T> & that) {
    std::swap(this->referent, that.referent);
}

template<typename T>
Collectible<T> & StrongRef<T>::operator*() const {
    assert(referent != nullptr);
    assert(referent->refCounter > 0);
    return *referent;
}

template<typename T>
Collectible<T> * StrongRef<T>::operator->() const {
    assert(referent != nullptr);
    assert(referent->refCounter > 0);
    return referent;
}
