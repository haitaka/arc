#include <cassert>
#include <iostream>
#include <sstream>
#include "mm.h"
#include "logger.h"

Collectible::Collectible() : refCounter(1) {}

void Collectible::incCounter() {
    auto prev = refCounter.fetch_add(1, std::memory_order_acq_rel);
    // Counter should have been initialized in constructor.
    // And it should be impossible to try to incCounter for the object,
    // that can die in a process (i.e. our caller should hold a strong ref to the object).
    assert(prev != 0);
    std::stringstream buf;
    buf << "Inc counter in " << this << " (was " << prev << ")" << std::endl;
    log(buf);
}

void Collectible::decCounter() {
    auto prev = refCounter.fetch_sub(1, std::memory_order_acq_rel);
    {
        std::stringstream buf;
        buf << "Dec counter in " << this << " (was " << prev << ")" << std::endl;
        log(buf);
    }
    if (prev == 1) {
        // free the object only if we were the thread that have seen the pre-zero counter
        {
            std::stringstream buf;
            buf << "Object " << this << " got dead" << std::endl;
            log(buf);
        }
        delete this;
    }
}

Collectible::~Collectible() {
    refCounter.store(0xBADBAD, std::memory_order_relaxed);
}

uint Collectible::getRefCounter() const {
    return refCounter.load(std::memory_order_relaxed); // not to synchronize with
}

RefToObj::RefToObj() : RefToObj(0) {}

RefToObj::RefToObj(std::size_t referent) : referent(referent) {}

RefToObj::RefToObj(RefToObj && that)  noexcept {
    auto thatVal = that.referent.exchange(0, std::memory_order_relaxed);
    referent.store(thatVal, std::memory_order_release);
}

RefToObj::RefToObj(RefToObj const & that) noexcept : RefToObj() {
    auto thatReferent = that.referent.load(std::memory_order_relaxed);
    auto thatPtr = clearWeakFlag(thatReferent);
    thatPtr->incCounter();
    referent.exchange(thatReferent, std::memory_order_relaxed);
}

RefToObj::~RefToObj() {
    auto taggedReferentPtr = referent.load(std::memory_order_relaxed);
    Collectible * collectible;
    if (taggedReferentPtr != 0) {
        if ((taggedReferentPtr & WEAK_TAG) != 0) {
            collectible = (WeakRef *) clearWeakFlag(taggedReferentPtr);
        } else {
            collectible = (Object *) taggedReferentPtr;
        }
        collectible->decCounter();
    }
}

RefToObj & RefToObj::operator=(RefToObj && that) noexcept {
    auto thatVal = that.referent.exchange(0, std::memory_order_relaxed);
    auto thisVal = referent.exchange(thatVal, std::memory_order_relaxed);
    that.referent.store(thisVal, std::memory_order_release);
    return *this;
}

RefToObj & RefToObj::operator=(RefToObj const & that) noexcept {
    if (this == &that) {
        return *this;
    }

    auto thatCopy = RefToObj(that);
    *this = RefToObj(that);

    return *this;
}

bool RefToObj::isEmpty() const {
    return referent.load(std::memory_order_relaxed) == 0;
}

bool RefToObj::isWeak() const {
    return (referent.load(std::memory_order_relaxed) & WEAK_TAG) != 0;
}

Object * RefToObj::get() const {
    auto taggedReferentPtr = referent.load();
    if ((taggedReferentPtr & WEAK_TAG) != 0) {
        auto weakRef = (WeakRef *) clearWeakFlag(taggedReferentPtr);
        auto obj = weakRef->referent.load(std::memory_order_acquire);
        if (obj == nullptr) {
            throw WeakRef::InvalidAccess();
        }
        return obj;
    } else {
        auto obj = (Object *) taggedReferentPtr;
        assert(obj != nullptr);
        return obj;
    }
}

Collectible * RefToObj::getRaw() const {
    return (Collectible * ) (clearWeakFlag(referent.load(std::memory_order_acquire)));
}

Object & RefToObj::operator*() const {
    return *get();
}

Object * RefToObj::operator->() const {
    return get();
}

Collectible * RefToObj::clearWeakFlag(std::size_t tagged) {
    return (Collectible *) (tagged & ~WEAK_TAG);
}

RefToObj RefToObj::newStrong(Object * obj) {
    std::stringstream buf;
    buf << "New strong ref to " << obj << std::endl;
    log(buf);
    assert(obj->getRefCounter() == 1);
    auto taggedReferentPtr = (std::size_t) obj;
    return RefToObj(taggedReferentPtr);
}

RefToObj RefToObj::makeStrong(RefToObj const & orig) {
    auto obj = orig.get();
    obj->incCounter();
    {
        std::stringstream buf;
        buf << "New strong ref to " << obj << std::endl;
        log(buf);
    }
    auto taggedReferentPtr = (std::size_t) obj;
    return RefToObj(taggedReferentPtr);
}

RefToObj RefToObj::makeWeak(RefToObj const & orig) {
    auto obj = orig.get();
    {
        std::stringstream buf;
        buf << "New weak ref to " << obj << std::endl;
        log(buf);
    }
    WeakRef * weakRef;
    if (obj->weakRef.isEmpty(std::memory_order_relaxed)) {
        weakRef = new WeakRef();
        {
            std::stringstream buf;
            buf << "Weak ref " << weakRef << " allocated" << std::endl;
            log(buf);
        }
        weakRef->referent.store(obj, std::memory_order_relaxed);
        obj->weakRef.referent.store(weakRef, std::memory_order_release);
    }
    weakRef = obj->weakRef.asPtr();
    weakRef->incCounter();
    auto taggedReferentPtr = (std::size_t) weakRef | WEAK_TAG;
    return RefToObj(taggedReferentPtr);
}

Global::~Global() {
    std::stringstream buf;
    buf << "Global " << this << " got dead" << std::endl;
    log(buf);
}

WeakRef::WeakRef() : referent(nullptr) {}

WeakRef::WeakRef(Object * referent) : referent(referent) {}

WeakRef::~WeakRef() {
    assert(referent.load(std::memory_order_relaxed) == nullptr);
}

void WeakRef::clear() {
    referent.store(nullptr, std::memory_order_release);
}

char const * WeakRef::InvalidAccess::what() const noexcept {
    return "dereferencing already freed weak reference";
}

Scope::NoSuchVar::NoSuchVar(char const * varKind, std::string const & varName) {
    std::stringstream buf;
    buf << "reading of undeclared " << varKind << " \"" << varName << "\"";
    descr = buf.str();
}

char const * Scope::NoSuchVar::what() const noexcept {
    return descr.c_str();
}

Globals::Globals(std::unordered_set<std::string> const & names) : globals() {
    for (auto & name : names) {
        globals[name]; // initialize all possible pairs to prevent future reallocations
    }
}

RefToObj Globals::get(std::string const & name) const {
    auto & refToGlobal = globals.at(name);
    if (refToGlobal.isEmpty(std::memory_order_acquire)) {
        throw NoSuchVar("global variable", name);
    }
    return refToGlobal->ref;
}

void Globals::put(std::string const & name, RefToObj && value) {
    auto & refToGlobal = globals.at(name);
    refToGlobal.initIfEmpty(); // first assignment is initialization
    refToGlobal->ref = std::move(value);
}

void Globals::erase(std::string const & name) {
    globals.at(name) = StrongRef<Global>();
}

Globals Globals::makeSubsetInitIfNeeded(std::unordered_set<std::string> const & names) {
    auto subset = Globals(names);
    for (auto & var : names) {
        StrongRef<Global> & x = globals.at(var);
        x.initIfEmpty();
        auto ptr = x.asPtr();
        assert(ptr != nullptr);
        subset.globals.at(var) = std::move(StrongRef<Global>::makeStrong(ptr));
    }
    return std::move(subset);
}

RefToObj Fields::get(std::string const & name) const {
    auto lock = std::lock_guard<std::mutex>(mutex);

    try {
        return fields.at(name);
    } catch (std::out_of_range & ex) {
        throw NoSuchVar("field", name);
    }
}

void Fields::put(std::string const & name, RefToObj && value) {
    auto lock = std::lock_guard<std::mutex>(mutex);
    fields.emplace(name, std::move(value));
}

std::unordered_map<std::string, Field> Fields::getMap() const {
    return {fields};
}

Object::Object(std::string const & name) : name(name), weakRef() {
    std::stringstream buf;
    buf << "New object " << name << "(" << this << ")" << std::endl;
    log(buf);
}

Object::~Object() {
    std::stringstream buf;
    buf << "Object " << name << "(" << this << ")" << " collected" << std::endl;
    log(buf);

    if (!weakRef.isEmpty(std::memory_order_relaxed)) {
        weakRef->clear();
    }
}

Fields & Object::getFields() {
    return fields;
}
