#include "Derived.hh"
#include "lib1.hh"

void lib1_run(const Base *b) {
    std::cout << "lib1_run ";
    attempt_cast(b);
}
