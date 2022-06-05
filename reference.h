#pragma once

#include <atomic>
#include <variant>
#include <unordered_map>

class AnyCollectible {

};

template<typename T>
class Collectible : public AnyCollectible {
public:
    Collectible(T && content);
    Collectible(Collectible<T> const & that) = delete;
    ~Collectible() = default;
    Collectible & operator =(Collectible<T> const & that) = delete;
private:
    T content;
    std::atomic<uint> refCounter = 0;
};

class AnyStrongRef {
};

template<typename T>
class StrongRef : public AnyStrongRef {
public:
    StrongRef();
    StrongRef(StrongRef const & that) = delete;
    StrongRef(StrongRef && that);
    ~StrongRef();
    void swap(StrongRef & that);
    StrongRef & operator =(StrongRef const & that) = delete;
    StrongRef & operator =(StrongRef && that);
protected:
    T referent;
};

class Var;

class StrongRefToVar final : public StrongRef<Collectible<Var>> {
public:
    StrongRefToVar(Collectible<Var> * referent);
    Collectible<Var> & operator *() const;
    Collectible<Var> * operator ->() const;
};

class Object;
class WeakRef;

// TODO doc uses tagged ptr
class StrongRefToObj final : public StrongRef<std::size_t> {
public:
    StrongRefToObj(Collectible<Object> * referent);
    Collectible<Object> & operator *() const;
    Collectible<Object> * operator ->() const;
};

// TODO comment
class WeakRef {
    WeakRef(Object * referent);
    // TODO assert no ref in destr?
    Object & operator *() const;
    Object * operator ->() const;
public:
private:
    Object * referent;
};

class Var {
public:
private:
    StrongRefToObj ref;
};

class Object {
public:
    Object() = default;
public:
    std::unordered_map<std::string, Var> fields; // TODO sync // TODO fast-write-sync
};


