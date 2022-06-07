#pragma once

#include <atomic>
#include <variant>
#include "ast.h"
#include "scope.h"

class AnyCollectible {

};

template<typename T>
class Collectible : public AnyCollectible {
public:
    Collectible() = default;
    Collectible(Collectible<T> const & that) = delete;
    ~Collectible() = default;
    Collectible & operator =(Collectible<T> const & that) = delete;
private:
    T content;
    std::atomic<uint> refCounter = 0;
};

template<typename T>
class StrongRef {
protected:
    explicit StrongRef(T referent);
public:
    StrongRef(StrongRef<T> const & that) = delete;
    StrongRef(StrongRef<T> && that);
    virtual ~StrongRef();
    StrongRef<T> & operator =(StrongRef<T> const & that) = delete;
    StrongRef<T> & operator =(StrongRef<T> && that);
    void swap(StrongRef<T> & that);
protected:
    T referent;
};

class Var;

class StrongRefToVar final : public StrongRef<Collectible<Var> *> { // TODO all the atomic stuff
public:
    StrongRefToVar(); // TODO constr/desrt
    explicit StrongRefToVar(Collectible<Var> * referent);
    StrongRefToVar(StrongRefToVar && that);
    ~StrongRefToVar() override;
    StrongRefToVar & operator =(StrongRefToVar && that);
    // Collectible<Var> & operator *() const;
    // Collectible<Var> * operator ->() const;
    Var * getRaw() const; // TODO consider *
};

class Object;
class WeakRef;

// TODO doc uses tagged ptr
class StrongRefToObj final : public StrongRef<std::size_t> {
public:
    StrongRefToObj();
    explicit StrongRefToObj(Collectible<Object> * referent);
    ~StrongRefToObj() override;
    // Collectible<Object> & operator *() const;
    // Collectible<Object> * operator ->() const;
    Collectible<Object> * get();
    void putStrong(Collectible<Object> * obj);
    void putWeak(Collectible<Object> * obj);
private:
    static std::size_t const WEAK_TAG = 1;
};

// TODO comment
class WeakRef {
public:
    WeakRef();
    explicit WeakRef(Collectible<Object> * referent);
    // TODO assert no ref in destr?
private:
    Collectible<Object> * referent;
};

class Var { // TODO remove
public:
    Var() = default;
    Var(Var && that);
    Var & operator=(Var && that);
    void swap(Var & that);
    Collectible<Object> * get();
    void putStrong(Collectible<Object> * obj);
    void putWeak(Collectible<Object> * obj);
private:
    StrongRefToObj ref;
};

class Object {
public:
    Object() = default;
    ~Object();
public:
    Scope<Var> fields;
    Collectible<WeakRef> * weakRef = nullptr;
};


template<typename T>
StrongRef<T>::StrongRef(T referent) : referent(referent) {}

template<typename T>
StrongRef<T>::StrongRef(StrongRef<T> && that) {
    swap(that);
}

template<typename T>
StrongRef<T>::~StrongRef() {} // FIXME not pure :c

template<typename T>
StrongRef<T> & StrongRef<T>::operator=(StrongRef<T> && that) {
    swap(that);
    return *this;
}

template<typename T>
void StrongRef<T>::swap(StrongRef<T> & that) {
    std::swap(referent, that.referent);
}
