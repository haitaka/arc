#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "common.h"
#include "ir.h"
#include "reference.h"

// FIXME capitalization
class Object {
public:
    Object() = default;
    Object(Object const & that) = delete;
    ~Object();
    Object & operator =(Object const & that) = delete;
public:
    std::unordered_map<std::string, Reference> fields;
    uint refCounter = 0; // TODO make atomic?
    std::unordered_set<Reference *> weakReferences;
};

