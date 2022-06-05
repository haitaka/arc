#include "object.h"

Object::~Object() {
    for (auto ref : weakReferences) {
        ref->referent = nullptr;
    }
}
