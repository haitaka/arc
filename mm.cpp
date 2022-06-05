#include <cassert>
#include <iostream>
#include "mm.h"

template<typename T>
Collectible<T>::Collectible(T && content) : content(content) {}


StrongRefToVar::StrongRefToVar()
        : StrongRefToVar(nullptr) {}

StrongRefToVar::StrongRefToVar(Collectible<Var> * referent)
        : StrongRef<Collectible<Var> *>(referent) {}

StrongRefToVar::~StrongRefToVar() {
    delete referent;
}

Var * StrongRefToVar::getRaw() const {
    return &referent->content;
}


StrongRefToObj::StrongRefToObj() : StrongRefToObj(nullptr) {

}

StrongRefToObj::StrongRefToObj(Collectible<Object> * referent)
        : StrongRef<std::size_t>((std::size_t) (referent)) {} // FIXME is it a bad cast?

StrongRefToObj::~StrongRefToObj() {
    // TODO generalize
    auto taggedReferentPtr = referent;
    if ((taggedReferentPtr & WEAK_TAG) != 0) {
        auto weakRef = (Collectible<WeakRef> *) (taggedReferentPtr | ~WEAK_TAG);
        delete weakRef;
    } else {
        auto referent = (Collectible<Object> *) taggedReferentPtr; // TODO dont hide
        delete referent;
    }
}

Object * StrongRefToObj::get() {
    auto taggedReferentPtr = referent;
    if ((taggedReferentPtr & WEAK_TAG) != 0) {
        auto weakRef = (Collectible<WeakRef> *) (taggedReferentPtr | ~WEAK_TAG);
        auto referent = weakRef->content.referent;
        if (referent == nullptr) {
            throw "NPE"; // TODO
        }
        return referent;
    } else {
        auto referent = (Collectible<Object> *) taggedReferentPtr;
        assert(referent != nullptr);
        return &referent->content;
    }
}

void StrongRefToObj::putStrong(Object * obj) {
    assert(false);
}

void StrongRefToObj::putWeak(Object * obj) {
    assert(false);
}


WeakRef::WeakRef(Object * referent)
        : referent(referent) {}

Object & WeakRef::operator*() const {
    return *referent;
}

Object * WeakRef::operator->() const {
    return referent;
}


Object * Var::get() {
    return ref.get();
}

void Var::putStrong(Object * obj) {
    ref.putStrong(obj);
}

void Var::putWeak(Object * obj) {
    ref.putWeak(obj);
}
