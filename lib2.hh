#ifndef LIB2_HH
#define LIB2_HH

#include "Base.hh"
#include <memory>

void EXPORT lib2_run(const Base *b);
std::unique_ptr<Base> EXPORT lib2_make_derived(int i);

#endif /* end of include guard: LIB2_HH */
