#include <cassert>
#include <iostream>
#include "reference.h"
#include "object.h"

Reference::Reference() : referent(nullptr), isStrong(false) {} // TODO used?

Reference::Reference(Object * referent, bool isStrong)
        : referent(referent)
        , isStrong(isStrong) {
    if (isStrong) {
        referent->refCounter += 1;
    } else {
        referent->weakReferences.insert(this);
    }
    auto kind = isStrong ? "strong" : "weak";
    std::clog << "New " << kind << " reference to " << referent
        << " // counter is now " << referent->refCounter << std::endl;
}

Reference::Reference(Reference const & existing, bool isStrong)
        : Reference(existing.referent, isStrong) {}

//Reference::Reference(Reference && that) noexcept
//        : Reference() {
//    swap(that);
//}

Reference::~Reference() {
    if (referent != nullptr) {
        auto kind = isStrong ? "strong" : "weak";
        uint counter = referent->refCounter;
        if (isStrong) {
            counter -= 1;
        }
        std::clog << "One less " << kind << " reference to " << referent;
        std::clog << " // counter is now " << counter;
        std::clog << std::endl;

        if (isStrong) {
            { // TODO synchronize
                referent->refCounter -= 1;
                if (referent->refCounter == 0) {
                    delete referent;
                    // TODO clean other weak references
                }
            }
        } else {
            referent->weakReferences.erase(this);
        }
    }
}

//Reference & Reference::operator=(Reference && that) noexcept {
//    swap(that);
//    return *this;
//}

//void Reference::swap(Reference & that) {
//    std::swap(this->referent, that.referent);
//    std::swap(this->isStrong, that.isStrong);
//}

Object & Reference::operator*() const {
    assert(referent->refCounter > 0);
    return *referent;
}

Object * Reference::operator->() const {
    assert(referent->refCounter > 0);
    return referent;
}

Object * Reference::get() const {
    assert(referent->refCounter > 0);
    return referent;
}
