#pragma once

#include <atomic>
#include <variant>
#include <mutex>
#include <unordered_map>
#include "ast.h"
#include "logger.h"

class Collectible {
    template<typename T>
    friend class StrongRef;
    friend class RefToObj;
public:
    Collectible();
    void incCounter();
    void decCounter();
    virtual ~Collectible();
    [[nodiscard]] uint getRefCounter() const;
private:
    std::atomic<uint> refCounter = 0;
};

template<typename T>
class StrongRef {
    friend class RefToObj;
public:
    StrongRef();
private:
    explicit StrongRef(T * referent);
public:
    StrongRef(StrongRef<T> const & that) = delete;
    StrongRef(StrongRef<T> && that) noexcept;
    virtual ~StrongRef();
    StrongRef<T> & operator =(StrongRef<T> const & that) = delete;
    StrongRef<T> & operator =(StrongRef<T> && that) noexcept;
    [[nodiscard]] bool isEmpty(std::memory_order order) const;
    void initIfEmpty();
    [[nodiscard]] T * asPtr() const;
    T & operator *() const;
    T * operator ->() const;
    static StrongRef<T> newStrong(T * referent);
    static StrongRef<T> makeStrong(T * referent);
private:
    std::atomic<T *> referent;
};

class Object;
class WeakRef;

/**
 * A reference to an object.
 * Can be strong or weak.
 * If strong, the `referent` field contains a pinter to the object.
 * If weak, the `referent` field contains a pointer to a `WeakRef` instance, marked with `WEAK_TAG`.
 */
class RefToObj { // FIXME pretty similar to `StrongRef`, find a way to generalize
public:
    RefToObj();
private:
    explicit RefToObj(std::size_t referent);
public:
    RefToObj(RefToObj && that) noexcept ;
    /**
     * Use with a great care!
     */
    RefToObj(RefToObj const & that) noexcept;
    ~RefToObj();
    RefToObj & operator =(RefToObj && that) noexcept ;
    /**
     * Use with a great care!
     */
    RefToObj & operator =(RefToObj const & that) noexcept;
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] bool isWeak() const;
    [[nodiscard]] Object * get() const;
    [[nodiscard]] Collectible * getRaw() const;
    Object & operator *() const;
    Object * operator ->() const;
    static RefToObj newStrong(Object * obj);
    static RefToObj makeStrong(RefToObj const & orig);
    static RefToObj makeWeak(RefToObj const & orig);
private:
    static Collectible * clearWeakFlag(std::size_t tagged);
    static std::size_t const WEAK_TAG = 1;
    std::atomic<std::size_t> referent;
};

class Global : public Collectible {
public:
    ~Global() override;
    RefToObj ref;
};

using Field = RefToObj;

/**
 * A proxy between the variable weak-referencing an object and the object.
 * Each object can be referenced by no more than one instance of `WeakRef`.
 * While actual vars contain strong references to this proxy object.
 * Thus, it's pretty simple to free the object's weak references when the object dies.
 */
class WeakRef : public Collectible {
    friend RefToObj;
public:
    WeakRef();
    explicit WeakRef(Object * referent);
    ~WeakRef() override;
    void clear();

    class InvalidAccess : public std::exception {
    public:
        [[nodiscard]] char const * what() const noexcept override;
    };
private:
    std::atomic<Object *> referent;
};

class Scope {
public:
    [[nodiscard]] virtual RefToObj get(std::string const & name) const = 0;
    virtual void put(std::string const & name, RefToObj && value) = 0;

    class NoSuchVar : public std::exception {
    public:
        NoSuchVar(char const * varKind, std::string const & varName);
        [[nodiscard]] char const * what() const noexcept override;
    private:
        std::string descr;
    };
};

class Globals : public Scope {
public:
    explicit Globals(std::unordered_set<std::string> const & names);
    [[nodiscard]] RefToObj get(std::string const & name) const override;
    void put(std::string const & name, RefToObj && value) override;
    void erase(std::string const & name);
    Globals makeSubsetInitIfNeeded(std::unordered_set<std::string> const & names);
private:
    std::unordered_map<std::string, StrongRef<Global>> globals;
};

class Fields : public Scope {
public:
    Fields() = default;
    RefToObj get(std::string const & name) const override;
    void put(std::string const & name, RefToObj && value) override;

    std::unordered_map<std::string, Field> getMap() const;
private:
    // a lock-free hash table would be a better choice, but it's to complex to spend time on
    mutable std::mutex mutex;
    std::unordered_map<std::string, Field> fields;
};

class Object : public Collectible {
    friend RefToObj;
public:
    explicit Object(std::string const & name);
    ~Object() override;

    Fields & getFields();
private:
    std::string name;
    Fields fields;
    StrongRef<WeakRef> weakRef;
};


template<typename T>
StrongRef<T>::StrongRef() : StrongRef(nullptr) {}

template<typename T>
StrongRef<T>::StrongRef(T * referent) : referent(referent) {
    if (referent != nullptr) {
        assert(referent->refCounter.load(std::memory_order_relaxed) > 0);
    }
}

template<typename T>
StrongRef<T>::StrongRef(StrongRef<T> && that) noexcept {
    auto thatVal = that.referent.exchange(nullptr, std::memory_order_relaxed);
    referent.store(thatVal, std::memory_order_release);
}

template<typename T>
StrongRef<T>::~StrongRef() {
    auto ptr = referent.load(std::memory_order_seq_cst);
    if (ptr != nullptr) {
        ptr->decCounter();
    }
}

template<typename T>
StrongRef<T> & StrongRef<T>::operator=(StrongRef<T> && that) noexcept {
    auto thatVal = that.referent.exchange(nullptr, std::memory_order_relaxed);
    auto thisVal = referent.exchange(thatVal, std::memory_order_relaxed);
    that.referent.store(thisVal, std::memory_order_release);
    return *this;
}

template<typename T>
bool StrongRef<T>::isEmpty(std::memory_order order) const {
    return referent.load(order) == nullptr;
}

template<typename T>
void StrongRef<T>::initIfEmpty() {
    if (isEmpty(std::memory_order_relaxed)) {
        T * emptyVal = nullptr;
        T * newValue = new T();
        bool newValueUsed = referent.compare_exchange_strong(emptyVal, newValue, std::memory_order_release, std::memory_order_relaxed);
        if (!newValueUsed) {
            delete newValue;
        }
    }
    assert(!isEmpty(std::memory_order_relaxed));
}

template<typename T>
T * StrongRef<T>::asPtr() const {
    return referent.load(std::memory_order_acquire);
}

template<typename T>
T & StrongRef<T>::operator*() const {
    return *asPtr();
}

template<typename T>
T * StrongRef<T>::operator->() const {
    return asPtr();
}

template<typename T>
StrongRef<T> StrongRef<T>::newStrong(T * referent) {
    return StrongRef<T>(referent);
}

template<typename T>
StrongRef<T> StrongRef<T>::makeStrong(T * referent) {
    referent->incCounter();
    return StrongRef<T>(referent);
}
