#ifndef DERIVED1_HH
#define DERIVED1_HH

#include "Base.hh"
#include <iostream>

struct EXPORT_CLASS Derived1 : public Base {
#if OUT_OF_LINE_DTOR
    virtual ~Derived1();
#else
    virtual ~Derived1() { }
#endif
};

struct EXPORT_CLASS Derived2 : public Base {
#if OUT_OF_LINE_DTOR
    virtual ~Derived2();
#else
    virtual ~Derived2() { }
#endif
};

struct EXPORT_CLASS Derived3 : public Derived2 {
    virtual ~Derived3() { }
};

inline void attempt_cast(const Base *b) {
    if      (dynamic_cast<const Derived1 *>(b)) { std::cout << "dynamic_cast Derived1" << std::endl; }
    else if (dynamic_cast<const Derived2 *>(b)) { std::cout << "dynamic_cast Derived2" << std::endl; }
    else { std::cout << "dynamic_cast failed: " << typeid(*b).name() << " vs " << typeid(Derived1).name() << " and " << typeid(Derived2).name() << std::endl; }
}

EXPORT std::unique_ptr<Base> make_derived(int i);

#endif /* end of include guard: DERIVED1_HH */
