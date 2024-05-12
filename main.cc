#include "lib1.hh"
#include "lib2.hh"
#include "Derived.hh"

#include <vector>

int main(int argc, const char *argv[]) {
    std::vector<std::unique_ptr<Base>> bases;
    bases.push_back(make_derived(1));
    bases.push_back(make_derived(2));
    bases.push_back(make_derived(3));

    for (const auto &b : bases) {
        lib1_run(b.get());
        lib2_run(b.get());

        auto b_ptr = b.get();
        std::cout << "main ";
        attempt_cast(b_ptr);
    }

    return 0;
}
