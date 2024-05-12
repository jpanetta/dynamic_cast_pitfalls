#include "Derived.hh"
#include "lib2.hh"

void lib2_run(const Base *b) {
    std::cout << "lib2_run ";
    attempt_cast(b);
}
