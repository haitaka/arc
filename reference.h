#pragma once

class Object;

class Reference {
public:
    Reference();
    Reference(Object * referent, bool isStrong);
    Reference(Reference const & existing, bool isStrong);
    Reference(Reference const & that) = delete;
    Reference(Reference && that) = delete;
    ~Reference();
    //void swap(Reference & that);
    Reference & operator =(Reference const & that) = delete;
    Reference & operator =(Reference && that) = delete;
    Object & operator *() const;
    Object * operator ->() const;
    Object * get() const; // TODO operator

private:
    bool isStrong;
    Object * referent;
};
