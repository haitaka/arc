#include <cassert>
#include <iostream>
#include "mm.h"


StrongRefToVar::StrongRefToVar()
        : StrongRefToVar(nullptr) {}

StrongRefToVar::StrongRefToVar(Collectible<Var> * referent)
        : StrongRef<Collectible<Var> *>(referent) {
    if (referent != nullptr) {
        referent->refCounter += 1;
    }
}

StrongRefToVar::StrongRefToVar(StrongRefToVar && that)
        : StrongRef<Collectible<Var> *>(std::move(that)) {}

StrongRefToVar::~StrongRefToVar() {
    if (referent != nullptr) {
        referent->refCounter -= 1; // FIXME race
        assert(referent->refCounter >= 0);
        if (referent->refCounter == 0) {
            std::clog << "Var " << referent << " got dead" << std::endl;
            delete referent;
        }
    }
}

StrongRefToVar & StrongRefToVar::operator=(StrongRefToVar && that) {
    StrongRef<Collectible<Var> *>::operator=(std::move(that));
    return *this;
}

Var * StrongRefToVar::getRaw() const {
    return &referent->content;
}


StrongRefToObj::StrongRefToObj() : StrongRefToObj(nullptr) {

}

StrongRefToObj::StrongRefToObj(Collectible<Object> * referent)
        : StrongRef<std::size_t>((std::size_t) (referent)) { // FIXME is it a bad cast?
    if (referent != nullptr) {
        referent->refCounter += 1;
    }
}

StrongRefToObj::~StrongRefToObj() {
    // TODO generalize
    auto taggedReferentPtr = referent;
    if (referent != 0) {
        if ((taggedReferentPtr & WEAK_TAG) != 0) {
            auto weakRef = (Collectible<WeakRef> *) (taggedReferentPtr & ~WEAK_TAG);
            weakRef->refCounter -= 1; // FIXME race
            assert(weakRef->refCounter >= 0);
            if (weakRef->refCounter == 0) {
                std::clog << "Weak ref to " << weakRef->content.referent << " got dead" << std::endl;
                delete weakRef;
            }
        } else {
            auto referent = (Collectible<Object> *) taggedReferentPtr; // TODO dont hide
            referent->refCounter -= 1; // FIXME race
            assert(referent->refCounter >= 0);
            if (referent->refCounter == 0) {
                std::clog << "Object " << referent << " got dead" << std::endl;
                delete referent;
            }
        }
    }
}

Collectible<Object> * StrongRefToObj::get() {
    auto taggedReferentPtr = referent;
    if ((taggedReferentPtr & WEAK_TAG) != 0) {
        auto weakRef = (Collectible<WeakRef> *) (taggedReferentPtr & ~WEAK_TAG);
        auto referent = weakRef->content.referent;
        if (referent == nullptr) {
            throw "NPE"; // TODO
        }
        return referent;
    } else {
        auto referent = (Collectible<Object> *) taggedReferentPtr;
        assert(referent != nullptr);
        return referent;
    }
}

void StrongRefToObj::putStrong(Collectible<Object> * obj) {
    std::clog << "New strong ref to " << obj << std::endl;
    auto taggedReferentPtr = (std::size_t) obj;
    obj->refCounter += 1;
    referent = taggedReferentPtr; // TODO atomic stuff
}

void StrongRefToObj::putWeak(Collectible<Object> * obj) {
    std::clog << "New weak ref to " << obj << std::endl;
    Collectible<WeakRef> * weakRef;
    if (obj->content.weakRef == nullptr) {
        weakRef = new Collectible<WeakRef>(); // TODO atomic
        weakRef->content.referent = obj;
        weakRef->refCounter += 1;
        obj->content.weakRef = weakRef;
    }
    auto taggedReferentPtr = (std::size_t) weakRef | WEAK_TAG;
    referent = taggedReferentPtr;
}


WeakRef::WeakRef() : referent(nullptr) {}

WeakRef::WeakRef(Collectible<Object> * referent)
        : referent(referent) {}

Var::Var(Var && that) {
    swap(that);
}

Var & Var::operator=(Var && that) {
    swap(that);
    return *this;
}

void Var::swap(Var & that) {
    ref.swap(that.ref);
}

Collectible<Object> * Var::get() {
    return ref.get();
}

void Var::putStrong(Collectible<Object> * obj) {
    ref.putStrong(obj);
}

void Var::putWeak(Collectible<Object> * obj) {
    ref.putWeak(obj);
}


Object::~Object() {
    if (weakRef != nullptr) {
        weakRef->content.referent = nullptr;
    }
}
