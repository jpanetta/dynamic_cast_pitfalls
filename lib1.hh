#ifndef LIB1_HH
#define LIB1_HH

#include "Base.hh"
#include <memory>

void EXPORT lib1_run(const Base *b);
std::unique_ptr<Base> EXPORT lib1_make_derived(int i);

#endif /* end of include guard: LIB1_HH */
