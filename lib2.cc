#include "Derived.hh"
#include "lib2.hh"

void lib2_run(const Base *b) {
    std::cout << "lib2_run ";
    attempt_cast(b);
}

std::unique_ptr<Base> lib2_make_derived(int i) {
    if (i == 1) return std::make_unique<Derived1>();
    if (i == 2) return std::make_unique<Derived2>();
    if (i == 3) return std::make_unique<Derived3>();
    throw std::runtime_error("invalid argument");
}
