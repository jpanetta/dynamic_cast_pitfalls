#include "Derived.hh"

#if OUT_OF_LINE_DTOR
Derived1::~Derived1() { }
Derived2::~Derived2() { }
#endif

std::unique_ptr<Base> make_derived(int i) {
    if (i == 1) return std::make_unique<Derived1>();
    if (i == 2) return std::make_unique<Derived2>();
    if (i == 3) return std::make_unique<Derived3>();
    throw std::runtime_error("invalid argument");
}
